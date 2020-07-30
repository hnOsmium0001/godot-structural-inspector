#include "Property.hpp"
#include "Schema.hpp"

#include <CheckBox.hpp>
#include <GlobalConstants.hpp>
#include <InputEvent.hpp>
#include <InputEventMouseButton.hpp>
#include <InputEventMouseMotion.hpp>
#include <JSON.hpp>
#include <MenuButton.hpp>
#include <PopupMenu.hpp>
#include <SpinBox.hpp>
#include <algorithm>
#include <utility>

using namespace godot;
using namespace godot::structural_inspector;

void ResourceSchema::_register_methods() {
	register_property("properties", &ResourceSchema::properties, Array{});
}

void ResourceSchema::_init() {
}

static std::unique_ptr<Schema> parse_schema(const Dictionary& def) {
	String type = def["type"];
	if (type == "struct") {
		auto schema = std::make_unique<StructSchema>();
		Array fields = def["fields"];
		for (int i = 0; i < fields.size(); ++i) {
			Dictionary property_def = fields[i];
			schema->fields.push_back({ property_def["name"], parse_schema(property_def) });
		}
		return schema;
	} else if (type == "array") {
		auto schema = std::make_unique<ArraySchema>();
		schema->element_type = parse_schema(def["element_type"]);
		return schema;
	} else if (type == "string") {
		auto schema = std::make_unique<StringSchema>();
		if (def.has("pattern")) {
			String pattern = def["pattern"];
			auto regex = RegEx::_new();
			if (regex->compile(pattern) != Error::OK) {
				return nullptr;
			}
			schema->pattern = Ref{ regex };
		}
		return schema;
	} else if (type == "enum") {
		auto schema = std::make_unique<EnumSchema>();
		Array values = def["values"];
		for (int i = 0; i < values.size(); ++i) {
			Dictionary value = values[i];
			schema->elements.push_back({ value["name"], value["id"] });
		}
		return schema;
	} else if (type == "int") {
		auto schema = std::make_unique<IntSchema>();
		schema->min_value = def["min_value"];
		schema->max_value = def["max_value"];
		return schema;
	} else if (type == "float") {
		auto schema = std::make_unique<FloatSchema>();
		schema->min_value = def["min_value"];
		schema->max_value = def["max_value"];
		return schema;
	} else if (type == "bool") {
		return std::make_unique<BoolSchema>();
	} else {
		return nullptr;
	}
}

const std::unordered_map<String, Schema*>& ResourceSchema::get_info() const {
	if (!initialized) {
		for (auto& [_, field] : info_cache) {
			delete field;
		}
		info_cache.clear();
		for (int i = 0; i < properties.size(); ++i) {
			Dictionary dict = properties[i];
			info_cache.insert({ dict["name"], ::parse_schema(dict).release() });
		}
		initialized = true;
	}
	return info_cache;
}

std::unordered_map<String, Schema*> ResourceSchema::get_info_copy() const {
	std::unordered_map<String, Schema*> copy;
	auto& source = get_info();
	for (auto& [name, schema] : source) {
		copy.insert({ name, schema->clone() });
	}
	return copy;
}

static Dictionary save_schema(Schema* schema) {
	Dictionary property;
	if (auto stru = dynamic_cast<StructSchema*>(schema)) {
		Array fields;
		for (auto& [name, field] : stru->fields) {
			auto field_dict = save_schema(field.get());
			field_dict["name"] = name;
			fields.append(field_dict);
		}
		property["type"] = "struct";
		property["fields"] = fields;
	} else if (auto array = dynamic_cast<ArraySchema*>(schema)) {
		property["type"] = "array";
		property["element_type"] = save_schema(array->element_type.get());
		property["min_elements"] = array->min_elements;
		property["max_elements"] = array->max_elements;
	} else if (auto string = dynamic_cast<StringSchema*>(schema)) {
		property["type"] = "string";
		if (string->pattern.is_valid()) {
			property["pattern"] = string->pattern->get_pattern();
		}
	} else if (auto en = dynamic_cast<EnumSchema*>(schema)) {
		Array values;
		for (auto& [name, id] : en->elements) {
			Dictionary value;
			value["name"] = name;
			value["id"] = id;
		}
		property["type"] = "enum";
		property["values"] = values;
	} else if (auto sint = dynamic_cast<IntSchema*>(schema)) {
		property["type"] = "int";
		property["min_value"] = sint->min_value;
		property["max_value"] = sint->max_value;
	} else if (auto sfloat = dynamic_cast<FloatSchema*>(schema)) {
		property["type"] = "int";
		property["min_value"] = sfloat->min_value;
		property["max_value"] = sfloat->max_value;
	} else if (auto sbool = dynamic_cast<BoolSchema*>(schema)) {
		property["type"] = "bool";
	}
	return property;
}

void ResourceSchema::set_info(Iterator<std::pair<String, Schema*>>& data) {
	properties.clear();
	while (data.has_next()) {
		auto curr = data.next();
		properties.append(save_schema(curr.second));
	}
	initialized = false;
}

ResourceSchema::ResourceSchema() {
}

ResourceSchema::~ResourceSchema() {
	for (auto& [_, schema] : info_cache) {
		delete schema;
	}
}

void StructEditor::_register_methods() {
	register_method("has_proerpty_name", &StructEditor::has_property_name);
	register_method("get_property_name", &StructEditor::get_property_name);
	register_method("set_property_name", &StructEditor::set_property_name);
}

void StructEditor::_init() {
	toolbar = HBoxContainer::_new();
	add_child(toolbar);

	title = Label::_new();
	toolbar->add_child(title);

	fields = VBoxContainer::_new();
	add_child(fields);
}

void StructEditor::_custom_init(const StructSchema* schema, CommonInspectorProperty* notif_target, const Array& path) {
	this->schema = schema;
	this->notif_target = notif_target;
	this->path = path;

	for (auto& [name, schema] : schema->fields) {
		auto path = this->path.duplicate();
		path.append(name);
		fields->add_child(schema->create_edit(&name, notif_target, path));
	}
}

bool StructEditor::has_property_name() const {
	return title->is_visible();
}

String StructEditor::get_property_name() const {
	return title->get_text();
}

void StructEditor::set_property_name(const String& name) {
	title->set_visible(true);
	title->set_text(name);
}

StructEditor::StructEditor() {
}

StructEditor::~StructEditor() {
}

void ArrayEditor::_element_gui_input(Ref<InputEvent> event, Control* element) {
	if (auto mb = Object::cast_to<InputEventMouseButton>(event.ptr())) {
		if (element->get_global_rect().has_point(get_global_mouse_position())) {
			selected_idx = element->get_index();
			remove->set_disabled(false);
			return;
		}
	}
}

void ArrayEditor::_add_element() {
	auto path = this->path;
	path.append(elements->get_child_count());
	auto element = schema->create_edit(nullptr, notif_target, path);
	notif_target->add_array_element(-1, schema->create_value(), path);
	elements->add_child(element);
	if (selected_idx != -1) {
		elements->move_child(element, selected_idx);
	}

	Array binds{};
	binds.append(element);
	element->connect("gui_input", this, "_element_gui_input", binds);

	if (elements->get_child_count() == schema->max_elements) {
		add->set_disabled(true);
	}
}

void ArrayEditor::_remove_element() {
	if (selected_idx != -1) {
		elements->get_child(selected_idx)->queue_free();
		notif_target->remove_array_element(selected_idx, path);
		selected_idx = -1;
		remove->set_disabled(true);

		if (elements->get_child_count() < schema->max_elements) {
			add->set_disabled(false);
		}
	}
}

void ArrayEditor::_register_methods() {
	register_method("_element_gui_input", &ArrayEditor::_element_gui_input);
	register_method("_add_element", &ArrayEditor::_add_element);
	register_method("_remove_element", &ArrayEditor::_remove_element);
	register_method("has_proerpty_name", &ArrayEditor::has_property_name);
	register_method("get_property_name", &ArrayEditor::get_property_name);
	register_method("set_property_name", &ArrayEditor::set_property_name);
}

void ArrayEditor::_init() {
	toolbar = HBoxContainer::_new();
	add_child(toolbar);

	title = Label::_new();
	toolbar->add_child(title);
	add = NXButton::_new();
	add->_custom_init("Add");
	add->connect("pressed", this, "_add_element");
	toolbar->add_child(add);
	remove = NXButton::_new();
	remove->_custom_init("Remove");
	remove->connect("pressed", this, "_remove_element");
	toolbar->add_child(remove);

	elements = VBoxContainer::_new();
	add_child(elements);
}

void ArrayEditor::_custom_init(const ArraySchema* schema, CommonInspectorProperty* notif_target, const Array& path) {
	this->schema = schema;
	this->notif_target = notif_target;
	this->path = path;

	for (int i = 0; i < schema->min_elements; ++i) {
		auto path = this->path.duplicate();
		path.append(i);
		elements->add_child(schema->create_edit(nullptr, notif_target, path));
	}
}

bool ArrayEditor::has_property_name() const {
	return title->is_visible();
}

String ArrayEditor::get_property_name() const {
	return title->get_text();
}

void ArrayEditor::set_property_name(const String& name) {
	title->set_visible(true);
	title->set_text(name);
}

ArrayEditor::ArrayEditor() {
}

ArrayEditor::~ArrayEditor() {
}

Variant CommonInspectorProperty::find_object(const Array& path, int distance) {
	// Either `Array` or `Dictionary`
	auto current_object = get_edited_object()->get(get_edited_property());
	for (int i = 0; i < path.size() - distance; ++i) {
		auto& prop_name = path[i];
		switch (prop_name.get_type()) {
			case Variant::STRING: {
				String name = prop_name;
				Dictionary dict = current_object;
				current_object = dict[name];
			} break;
			case Variant::INT: {
				int idx = prop_name;
				Array array = current_object;
				current_object = array[idx];
			} break;
			default: {
				// Invalid `prop_name` type
				return Variant{ static_cast<Object*>(nullptr) };
			}
		}
	}
	return current_object;
}

void CommonInspectorProperty::_toggle_editor_visibility() {
	if (editor->is_visible()) {
		remove_child(editor);
		set_bottom_editor(nullptr);
		editor->set_visible(false);
	} else {
		add_child(editor);
		set_bottom_editor(editor);
		editor->set_visible(true);
	}
}

void CommonInspectorProperty::_register_methods() {
	register_method("_toggle_editor_visibility", &CommonInspectorProperty::_toggle_editor_visibility);
	register_method("update_property", &CommonInspectorProperty::update_property);
}

void CommonInspectorProperty::_init() {
	btn = Button::_new();
	btn->connect("pressed", this, "_toggle_editor_visibility");
	add_child(btn);
}

void CommonInspectorProperty::_custom_init(const Schema* schema, Control* editor) {
	this->schema = schema;
	this->editor = editor;

	editor->set_visible(false);
}

void CommonInspectorProperty::update_value(Variant value, const Array& path) {
	if (updating) return;

	auto object = find_object(path, 1);
	auto last_prop_name = path[path.size() - 1];
	switch (last_prop_name.get_type()) {
		case Variant::STRING: {
			String name = last_prop_name;
			Dictionary dict = object;
			dict[name] = value;
		} break;
		case Variant::INT: {
			int idx = last_prop_name;
			Array array = object;
			array[idx] = value;
		} break;
		default: {
			// Invalid `prop_name` type
			return;
		} break;
	}

	emit_changed(get_edited_property(), get_edited_object()->get(get_edited_property()), "", true);
}

void CommonInspectorProperty::add_array_element(int pos, Variant elm, const Array& path) {
	if (updating) return;

	Array array = find_object(path, 0);
	if (pos == -1) {
		array.insert(pos, elm);
	} else {
		array.append(elm);
	}

	emit_changed(get_edited_property(), get_edited_object()->get(get_edited_property()), "", true);
}

void CommonInspectorProperty::remove_array_element(int pos, const Array& path) {
	if (updating) return;

	Array array = find_object(path, 0);
	array.remove(pos);

	emit_changed(get_edited_property(), get_edited_object()->get(get_edited_property()), "", true);
}

void CommonInspectorProperty::update_property() {
	updating = true;
	schema->update_edit(editor, get_edited_object()->get(get_edited_property()));
	updating = false;
}

CommonInspectorProperty::CommonInspectorProperty() {
}

CommonInspectorProperty::~CommonInspectorProperty() {
}

static const int LABEL_POS = 0;
static const int EDIT_POS = 1;

static HBoxContainer* create_nc(const String& name, Control* control) {
	auto line = HBoxContainer::_new();
	auto label = Label::_new();
	label->set_text(name);
	label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	line->add_child(label);
	control->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	line->add_child(control);
	return line;
}

static HBoxContainer* get_nc_line(Control* control) {
	return static_cast<HBoxContainer*>(control->get_parent());
}

static Label* get_nc_label(Control* control) {
	auto parent = static_cast<HBoxContainer*>(control->get_parent());
	return static_cast<Label*>(parent->get_child(LABEL_POS));
}

static VBoxContainer* create_enum_value_edit(const String& name, int id, Object* target) {
	auto enum_value = VBoxContainer::_new();
	auto binding = Array::make(enum_value);
	auto name_edit = LineEdit::_new();
	name_edit->set_text(name);
	name_edit->connect("text_changed", target, "_enum_id_set", binding);
	enum_value->add_child(::create_nc("Name", name_edit));
	auto id_edit = SpinBox::_new();
	id_edit->set_max(std::numeric_limits<int>::max());
	id_edit->set_value(id);
	id_edit->connect("value_changed", target, "_enum_id_set", binding);
	enum_value->add_child(::create_nc("ID", id_edit));
	return enum_value;
}

ResourceSchemaNode* ResourceSchemaNode::_make_child() {
	auto child = ResourceSchemaNode::_new();
	child->_custom_init(root, this);
	auto binding = Array::make(child);
	child->connect("schema_changed", this, "_child_schema_changed", binding);
	child->connect("field_name_changed", this, "_child_field_name_changed", binding);
	child->connect("clicked", this, "_child_clicked", binding);
	return child;
}

bool ResourceSchemaNode::_is_mouse_inside() {
	return get_global_rect().has_point(get_global_mouse_position());
}

void ResourceSchemaNode::_type_selected_swap(int id, Schema* swap_out) {
	if (schema_id == id) {
		return;
	}

	static const auto free_all = [](Node* node) {
		for (int i = 0; i < node->get_child_count(); ++i) {
			node->get_child(i)->queue_free();
		}
	};

	switch (schema_id) {
		case STRUCT: {
			add->set_visible(false);
			remove->set_visible(false);
			message->set_visible(false);
			free_all(list);
			list->set_visible(false);
		} break;
		case ARRAY: {
			free_all(list);
			list->set_visible(false);
			::get_nc_line(min_value)->set_visible(false);
			::get_nc_line(max_value)->set_visible(false);
		} break;
		case STRING: {
			::get_nc_line(pattern)->set_visible(false);
		} break;
		case ENUM: {
			add->set_visible(false);
			remove->set_visible(false);
			message->set_visible(false);
			free_all(list);
			list->set_visible(false);
		} break;
		case INT: {
			::get_nc_line(min_value)->set_visible(false);
			::get_nc_line(max_value)->set_visible(false);
		} break;
		case FLOAT: {
			::get_nc_line(min_value)->set_visible(false);
			::get_nc_line(max_value)->set_visible(false);
		} break;
		case BOOL: {
			// Do nothing
		} break;
		default: {
			// Do nothing, for schema_id == UNKNOWN
		} break;
	}
	if (owns_schema) {
		delete schema;
		schema = nullptr;
	}

	switch (id) {
		case STRUCT: {
			add->set_visible(true);
			remove->set_visible(true);
			message->set_visible(true);
			list->set_visible(true);

			schema = swap_out ? swap_out : new StructSchema();
			schema_id = STRUCT;
			emit_signal("schema_changed");
		} break;
		case ARRAY: {
			list->set_visible(true);

			::get_nc_label(min_value)->set_text("Min elements");
			::get_nc_line(min_value)->set_visible(true);
			::get_nc_label(max_value)->set_text("Max elements");
			::get_nc_line(max_value)->set_visible(true);

			auto array = swap_out ? dynamic_cast<ArraySchema*>(swap_out) : new ArraySchema();
			schema = array;
			schema_id = ARRAY;
			// This will immediately emit a `schema_changed` signal (caused by `add_child` -> ready), we need our schema setup before that
			list->add_child(_make_child());
			min_value->set_value(array->min_elements);
			max_value->set_value(array->max_elements);
			emit_signal("schema_changed");
		} break;
		case STRING: {
			::get_nc_line(pattern)->set_visible(true);

			schema = swap_out ? swap_out : new StringSchema();
			schema_id = STRING;
			emit_signal("schema_changed");
		} break;
		case ENUM: {
			add->set_visible(true);
			remove->set_visible(true);
			message->set_visible(true);
			list->set_visible(true);

			schema = swap_out ? swap_out : new EnumSchema();
			schema_id = ENUM;
			emit_signal("schema_changed");
		} break;
		case INT: {
			::get_nc_label(min_value)->set_text("Min value");
			::get_nc_line(min_value)->set_visible(true);
			::get_nc_label(max_value)->set_text("Max value");
			::get_nc_line(max_value)->set_visible(true);

			auto sint = swap_out ? dynamic_cast<IntSchema*>(swap_out) : new IntSchema();
			schema = sint;
			schema_id = INT;
			min_value->set_value(sint->min_value);
			max_value->set_value(sint->max_value);
			emit_signal("schema_changed");
		} break;
		case FLOAT: {
			::get_nc_label(min_value)->set_text("Min value");
			::get_nc_line(min_value)->set_visible(true);
			::get_nc_label(max_value)->set_text("Max value");
			::get_nc_line(max_value)->set_visible(true);

			auto sfloat = swap_out ? dynamic_cast<FloatSchema*>(swap_out) : new FloatSchema();
			schema = sfloat;
			schema_id = FLOAT;
			min_value->set_value(sfloat->min_value);
			max_value->set_value(sfloat->max_value);
			emit_signal("schema_changed");
		} break;
		case BOOL: {
			schema = swap_out ? swap_out : new BoolSchema();
			schema_id = BOOL;
			emit_signal("schema_changed");
		} break;
		default: {
			schema = nullptr;
			schema_id = UNKNOWN;
			ERR_PRINT("Unknown type id " + String::num_int64(id));
		} break;
	}

	schema_id = static_cast<NodeType>(id);
	root->emit_something_changed();
}

void ResourceSchemaNode::_input(Ref<InputEvent> event) {
	if (auto mb = Object::cast_to<InputEventMouseButton>(event.ptr())) {
		if (mb->get_button_index() == GlobalConstants::BUTTON_LEFT && mb->is_pressed() && _is_mouse_inside()) {
			emit_signal("clicked");
		}
		return;
	}
	if (auto mm = Object::cast_to<InputEventMouseMotion>(event.ptr())) {
		if (bool current = _is_mouse_inside(); mouse_inside != current) {
			mouse_inside = current;
			update();
		}
		return;
	}
}

void ResourceSchemaNode::_notification(int what) {
	switch (what) {
		case NOTIFICATION_READY: {
			// When this schema node is restored from a .tres file, update_with() will be called before _ready()
			if (schema_id == UNKNOWN) {
				_type_selected(STRUCT);
				type_edit->select(STRUCT);
			}

			if (parent && parent->schema_id == STRUCT) {
				::get_nc_line(field_name)->set_visible(true);
			}
		} break;

		case NOTIFICATION_RESIZED: {
			update();
		} break;
		case NOTIFICATION_DRAW: {
			// First is regular border color, second is hovered border color
			static const Color colors[] = { Color::hex(0x262C3BFF), Color::hex(0x333B4FFF) };
			draw_rect(Rect2{ 4, 4, get_size().x - 4 * 2, get_size().y - 4 * 2 }, colors[mouse_inside], false, 2.0F);
		} break;
		default: {
		} break;
	}
}

void ResourceSchemaNode::_type_selected(int id) {
	_type_selected_swap(id, nullptr);
}

void ResourceSchemaNode::_add_list_item() {
	switch (schema_id) {
		case STRUCT: {
			auto schema = dynamic_cast<StructSchema*>(this->schema);
			schema->fields.push_back({ "", nullptr });

			auto child = _make_child();
			list->add_child(child);

			root->emit_something_changed();
		} break;
		case ENUM: {
			int idx = list->get_child_count(); // Is equal to `enum_value->get_index()`
			auto schema = dynamic_cast<EnumSchema*>(this->schema);
			// Default ID is its index
			schema->elements.push_back({ "", idx });

			auto enum_value = ::create_enum_value_edit("", idx, this);
			list->add_child(enum_value);

			root->emit_something_changed();
		}
		default: {
			return;
		}
	}
}

void ResourceSchemaNode::_toggle_remove_mode() {
	switch (schema_id) {
		case STRUCT:
		case ENUM: {
			if (removing_child) {
				message->set_text("");
				removing_child = false;
			} else {
				message->set_text("Click a child property to remove.");
				removing_child = true;
			}
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaNode::_field_name_set(const String& field_name) {
	emit_signal("field_name_changed", field_name);
	root->emit_something_changed();
}

void ResourceSchemaNode::_min_value_set(real_t value) {
	switch (schema_id) {
		case ARRAY: {
			auto schema = dynamic_cast<ArraySchema*>(this->schema);
			schema->min_elements = static_cast<int>(value);
			root->emit_something_changed();
		} break;
		case INT: {
			auto schema = dynamic_cast<IntSchema*>(this->schema);
			schema->min_value = static_cast<int>(value);
			root->emit_something_changed();
		} break;
		case FLOAT: {
			auto schema = dynamic_cast<FloatSchema*>(this->schema);
			schema->min_value = value;
			root->emit_something_changed();
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaNode::_max_value_set(real_t value) {
	switch (schema_id) {
		case ARRAY: {
			auto schema = dynamic_cast<ArraySchema*>(this->schema);
			schema->max_elements = static_cast<int>(value);
			root->emit_something_changed();
		} break;
		case INT: {
			auto schema = dynamic_cast<IntSchema*>(this->schema);
			schema->max_value = static_cast<int>(value);
			root->emit_something_changed();
		} break;
		case FLOAT: {
			auto schema = dynamic_cast<FloatSchema*>(this->schema);
			schema->max_value = value;
			root->emit_something_changed();
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaNode::_pattern_set(const String& pattern) {
	switch (schema_id) {
		case STRING: {
			auto schema = dynamic_cast<StringSchema*>(this->schema);
			schema->pattern = Ref{ RegEx::_new() };
			root->emit_something_changed();
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaNode::_child_schema_changed(ResourceSchemaNode* child) {
	switch (schema_id) {
		case STRUCT: {
			auto schema = dynamic_cast<StructSchema*>(this->schema);
			schema->fields[child->get_index()].def = std::unique_ptr<Schema>(child->take_schema());
		} break;
		case ARRAY: {
			auto schema = dynamic_cast<ArraySchema*>(this->schema);
			schema->element_type = std::unique_ptr<Schema>(child->take_schema());
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaNode::_child_field_name_changed(const String& new_name, ResourceSchemaNode* node) {
	switch (schema_id) {
		case STRUCT: {
			auto schema = dynamic_cast<StructSchema*>(this->schema);
			schema->fields[node->get_index()].name = new_name;
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaNode::_child_clicked(ResourceSchemaNode* child) {
	if (!removing_child) {
		return;
	}

	bool removed = false;
	switch (schema_id) {
		case STRUCT: {
			auto schema = dynamic_cast<StructSchema*>(this->schema);
			child->queue_free();
			schema->fields.erase(schema->fields.begin() + child->get_index());
			removed = true;
		} break;
		case ENUM: {
			auto schema = dynamic_cast<EnumSchema*>(this->schema);
			child->queue_free();
			schema->elements.erase(schema->elements.begin() + child->get_index());
			removed = true;

			root->emit_something_changed();
		} break;
		default: {
			return;
		}
	}

	if (removed) {
		message->set_text("");
		removing_child = false;

		root->emit_something_changed();
	}
}

void ResourceSchemaNode::_enum_name_set(const String& name, Control* child) {
	if (auto schema = dynamic_cast<EnumSchema*>(this->schema)) {
		schema->elements[child->get_index()].name = name;
		root->emit_something_changed();
	}
}

void ResourceSchemaNode::_enum_id_set(int id, Control* child) {
	if (auto schema = dynamic_cast<EnumSchema*>(this->schema)) {
		schema->elements[child->get_index()].id = id;
		root->emit_something_changed();
	}
}

void ResourceSchemaNode::_register_methods() {
	register_method("_input", &ResourceSchemaNode::_input);
	register_method("_notification", &ResourceSchemaNode::_notification);
	register_method("_type_selected", &ResourceSchemaNode::_type_selected);
	register_method("_add_list_item", &ResourceSchemaNode::_add_list_item);
	register_method("_toggle_remove_mode", &ResourceSchemaNode::_toggle_remove_mode);
	register_method("_field_name_set", &ResourceSchemaNode::_field_name_set);
	register_method("_min_value_set", &ResourceSchemaNode::_min_value_set);
	register_method("_max_value_set", &ResourceSchemaNode::_max_value_set);
	register_method("_pattern_set", &ResourceSchemaNode::_pattern_set);
	register_method("_child_schema_changed", &ResourceSchemaNode::_child_schema_changed);
	register_method("_child_field_name_changed", &ResourceSchemaNode::_child_field_name_changed);
	register_method("_child_clicked", &ResourceSchemaNode::_child_clicked);
	register_method("_enum_name_set", &ResourceSchemaNode::_enum_name_set);
	register_method("_enum_id_set", &ResourceSchemaNode::_enum_id_set);

	register_signal<ResourceSchemaNode>("schema_changed", Dictionary{});
	register_signal<ResourceSchemaNode>("field_name_changed", "name", Variant::STRING);
	register_signal<ResourceSchemaNode>("clicked", Dictionary{});
}

void ResourceSchemaNode::_init() {
	add_constant_override("margin_left", 8);
	add_constant_override("margin_top", 8);
	add_constant_override("margin_right", 8);
	add_constant_override("margin_bottom", 8);

	contents = VBoxContainer::_new();
	add_child(contents);

	type_edit = OptionButton::_new();
	type_edit->get_popup()->add_item("Struct", STRUCT);
	type_edit->get_popup()->add_item("Array", ARRAY);
	type_edit->get_popup()->add_item("String", STRING);
	type_edit->get_popup()->add_item("Enum", ENUM);
	type_edit->get_popup()->add_item("Int", INT);
	type_edit->get_popup()->add_item("Float", FLOAT);
	type_edit->get_popup()->add_item("Bool", BOOL);
	type_edit->connect("item_selected", this, "_type_selected");
	contents->add_child(::create_nc("Type", type_edit));

	field_name = LineEdit::_new();
	field_name->connect("text_changed", this, "_field_name_set");
	auto field_name_line = ::create_nc("Field name", field_name);
	field_name_line->set_visible(false);
	contents->add_child(field_name_line);

	toolbar = HBoxContainer::_new();
	contents->add_child(toolbar);
	add = NXButton::_new();
	add->_custom_init("Add");
	add->set_visible(false);
	add->connect("pressed", this, "_add_list_item");
	toolbar->add_child(add);
	remove = NXButton::_new();
	remove->_custom_init("Remove");
	remove->set_visible(false);
	remove->connect("pressed", this, "_toggle_remove_mode");
	toolbar->add_child(remove);
	message = Label::_new();
	message->set_visible(false);
	toolbar->add_child(message);

	min_value = SpinBox::_new();
	min_value->set_min(std::numeric_limits<int>::min());
	min_value->set_max(std::numeric_limits<int>::max());
	min_value->connect("value_changed", this, "_min_value_set");
	auto min_value_line = ::create_nc("Min value", min_value);
	min_value_line->set_visible(false);
	contents->add_child(min_value_line);

	max_value = SpinBox::_new();
	max_value->set_min(std::numeric_limits<int>::min());
	max_value->set_max(std::numeric_limits<int>::max());
	max_value->connect("value_changed", this, "_max_value_set");
	auto max_value_line = ::create_nc("Max value", max_value);
	max_value_line->set_visible(false);
	contents->add_child(max_value_line);

	pattern = LineEdit::_new();
	pattern->connect("text_changed", this, "_pattern_set");
	auto pattern_line = ::create_nc("Pattern", pattern);
	pattern_line->set_visible(false);
	contents->add_child(pattern_line);

	list = VBoxContainer::_new();
	list->set_visible(false);
	auto padding_box = MarginContainer::_new();
	padding_box->add_constant_override("margin_right", 0);
	padding_box->add_constant_override("margin_top", 0);
	padding_box->add_constant_override("margin_left", 16);
	padding_box->add_constant_override("margin_bottom", 0);
	padding_box->add_child(list);
	contents->add_child(padding_box);
}

void ResourceSchemaNode::_custom_init(ResourceSchemaInspectorProperty* root, ResourceSchemaNode* parent) {
	this->root = root;
	this->parent = parent;
}

void ResourceSchemaNode::update_with(Schema* schema) {
	if (auto other = dynamic_cast<StructSchema*>(schema)) {
		// We take ownership of this schema now
		_type_selected_swap(STRUCT, other);
		type_edit->select(STRUCT);

		for (int i = 0; i < list->get_child_count(); ++i) {
			list->get_child(i)->free();
		}
		for (auto& field : other->fields) {
			auto child = _make_child();
			// update_with() takes the ownership of parameter `schema`. But it will also fire signal `schema_changed`, causing us to take the ownership from it
			// Use release() instead of get() to prevent us freeing the pointer while taking the ownership back
			child->update_with(field.def.release());
			child->set_field_name(field.name);
			list->add_child(child);
		}
	} else if (auto other = dynamic_cast<ArraySchema*>(schema)) {
		_type_selected_swap(ARRAY, other);
		type_edit->select(ARRAY);

		min_value->set_value(other->min_elements);
		max_value->set_value(other->max_elements);
		get_child_node(0)->update_with(other->element_type.release());
	} else if (auto other = dynamic_cast<StringSchema*>(schema)) {
		_type_selected_swap(STRING, other);
		type_edit->select(STRING);

		if (other->pattern.is_valid()) {
			pattern->set_text(other->pattern->get_pattern());
		}
	} else if (auto other = dynamic_cast<EnumSchema*>(schema)) {
		_type_selected_swap(ENUM, other);
		type_edit->select(ENUM);

		for (int i = 0; i < list->get_child_count(); ++i) {
			list->get_child(i)->free();
		}
		for (auto& [name, id] : other->elements) {
			list->add_child(::create_enum_value_edit(name, id, this));
		}
	} else if (auto other = dynamic_cast<IntSchema*>(schema)) {
		_type_selected_swap(INT, other);
		type_edit->select(INT);

		min_value->set_value(other->min_value);
		max_value->set_value(other->max_value);
	} else if (auto other = dynamic_cast<FloatSchema*>(schema)) {
		_type_selected_swap(FLOAT, other);
		type_edit->select(FLOAT);

		min_value->set_value(other->min_value);
		max_value->set_value(other->max_value);
	} else if (auto other = dynamic_cast<BoolSchema*>(schema)) {
		_type_selected_swap(BOOL, other);
		type_edit->select(BOOL);
	}
}

String ResourceSchemaNode::get_field_name() const {
	return field_name->get_text();
}

void ResourceSchemaNode::set_field_name(const String& name) {
	::get_nc_line(field_name)->set_visible(true);
	field_name->set_text(name);
}

ResourceSchemaNode* ResourceSchemaNode::get_child_node(int i) {
	return Object::cast_to<ResourceSchemaNode>(list->get_child(i));
}

Schema* ResourceSchemaNode::get_schema() {
	return schema;
}

Schema* ResourceSchemaNode::take_schema() {
	owns_schema = false;
	return schema;
}

ResourceSchemaNode::ResourceSchemaNode() {
}

ResourceSchemaNode::~ResourceSchemaNode() {
	if (owns_schema) {
		delete schema;
	}
}

void ResourceSchemaInspectorProperty::_toggle_editor_visibility() {
	if (properties->is_visible()) {
		remove_child(properties);
		set_bottom_editor(nullptr);
		properties->set_visible(false);
	} else {
		add_child(properties);
		set_bottom_editor(properties);
		properties->set_visible(true);
	}
}

void ResourceSchemaInspectorProperty::_prop_clicked(ResourceSchemaNode* node) {
	selected_idx = node->get_index();
}

void ResourceSchemaInspectorProperty::_register_methods() {
	register_method("_toggle_editor_visibility", &ResourceSchemaInspectorProperty::_toggle_editor_visibility);
	register_method("_prop_clicked", &ResourceSchemaInspectorProperty::_prop_clicked);
	register_method("add_root_property", &ResourceSchemaInspectorProperty::add_root_property);
	register_method("remove_root_property", &ResourceSchemaInspectorProperty::remove_root_property);
	register_method("emit_something_changed", &ResourceSchemaInspectorProperty::emit_something_changed);
	register_method("update_property", &ResourceSchemaInspectorProperty::update_property);
}

void ResourceSchemaInspectorProperty::_init() {
	btn = Button::_new();
	btn->set_text("Properties");
	btn->connect("pressed", this, "_toggle_editor_visibility");
	add_child(btn);

	properties = VBoxContainer::_new();
	properties->set_visible(false);

	auto toolbar = HBoxContainer::_new();
	auto add = NXButton::_new();
	add->_custom_init("Add");
	add->connect("pressed", this, "add_root_property");
	toolbar->add_child(add);
	auto remove = NXButton::_new();
	remove->_custom_init("Remove");
	remove->connect("pressed", this, "remove_root_property");
	toolbar->add_child(remove);

	properties->add_child(toolbar);
}

ResourceSchemaNode* ResourceSchemaInspectorProperty::add_root_property() {
	auto prop = ResourceSchemaNode::_new();
	prop->_custom_init(this, nullptr);
	// Force enable field name
	prop->set_field_name("");
	prop->connect("clicked", this, "_prop_clicked", Array::make(prop));
	properties->add_child(prop);
	emit_something_changed();
	return prop;
}

void ResourceSchemaInspectorProperty::remove_root_property() {
	if (selected_idx != -1) {
		properties->get_child(selected_idx)->free();
		selected_idx = -1;
	}
	emit_something_changed();
}

void ResourceSchemaInspectorProperty::emit_something_changed() {
	if (updating) return;

	Array saved_props;
	for (int i = 1; i < properties->get_child_count(); ++i) {
		if (auto prop = Object::cast_to<ResourceSchemaNode>(properties->get_child(i))) {
			auto data = save_schema(prop->get_schema());
			data["name"] = prop->get_field_name();
			saved_props.append(data);
		} else {
			ERR_PRINT("Properties box contains unexpected nodes that aren't of type ResourceSchemaNode");
		}
	}

	emit_changed(get_edited_property(), saved_props, "", true);
}

void ResourceSchemaInspectorProperty::update_property() {
	updating = true;

	for (int i = 1; i < properties->get_child_count(); ++i) {
		properties->get_child(i)->free();
	}

	Array data = get_edited_object()->get(get_edited_property());
	for (int i = 0; i < data.size(); ++i) {
		Dictionary dict = data[i];
		// If parsing schema failed, we skip this entry
		if (auto schema = ::parse_schema(dict)) {
			auto prop = ResourceSchemaNode::_new();
			prop->_custom_init(this, nullptr);
			prop->update_with(schema.release());
			prop->set_field_name(dict["name"]);
			prop->connect("clicked", this, "_prop_clicked", Array::make(prop));
			properties->add_child(prop);
		} else {
			ERR_PRINT("Error while parsing schema entry: " + JSON::get_singleton()->print(dict));
		}
	}

	updating = false;
}

ResourceSchemaInspectorProperty::ResourceSchemaInspectorProperty() {
}

ResourceSchemaInspectorProperty::~ResourceSchemaInspectorProperty() {
}
