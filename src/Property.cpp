#include "Property.hpp"

#include <CheckBox.hpp>
#include <GlobalConstants.hpp>
#include <InputEvent.hpp>
#include <InputEventMouseButton.hpp>
#include <InputEventMouseMotion.hpp>
#include <MenuButton.hpp>
#include <PopupMenu.hpp>
#include <Texture.hpp>
#include <utility>

using namespace godot;
using namespace godot::structural_editor;

void NXButton::_notification(int what) {
	switch (what) {
		case NOTIFICATION_READY: {
			// Godot::print("ready");
			call_deferred("_apply_icon");
		} break;
		default: {
		} break;
	}
}

void NXButton::_apply_icon() {
	// Godot::print(icon_name);
	set_button_icon(get_icon(icon_name, "EditorIcons"));
}

void NXButton::_register_methods() {
	register_method("_notification", &NXButton::_notification);
	register_method("_apply_icon", &NXButton::_apply_icon);
}

void NXButton::_init() {
	set_flat(true);
}

void NXButton::_custom_init(const String& icon_name) {
	this->icon_name = icon_name;
}

NXButton::NXButton() {
}

NXButton::~NXButton() {
}

Variant StructSchema::create_value() const {
	return Dictionary::make();
}

Control* StructSchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	auto editor = StructEditor::_new();
	editor->_custom_init(this, target, path);
	if (name) {
		editor->set_property_name(*name);
	}
	return editor;
}

void StructSchema::update_edit(Control* edit, const Variant& data) const {
	auto editor = static_cast<StructEditor*>(edit);
	Dictionary data_source = data;
	for (int i = 0; i < fields.size(); ++i) {
		auto& field = fields[i];
		field.def->update_edit(
				static_cast<Control*>(editor->fields->get_child(i)),
				data_source[field.name]);
	}
}

EditorProperty* StructSchema::create_property() const {
	auto prop = CommonInspectorProperty::_new();
	auto editor = StructEditor::_new();
	editor->_custom_init(this, prop, Array::make());
	prop->_custom_init(this, editor);
	return prop;
}

Variant ArraySchema::create_value() const {
	return Array::make();
}

Control* ArraySchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	auto editor = ArrayEditor::_new();
	editor->_custom_init(this, target, path);
	if (name) {
		editor->set_property_name(*name);
	}
	return editor;
}

void ArraySchema::update_edit(Control* edit, const Variant& data) const {
	auto editor = static_cast<ArrayEditor*>(data);
	Array data_source = data;
	for (int i = 0; i < data_source.size(); ++i) {
		auto field = static_cast<Control*>(editor->elements->get_child(i));
		element_type->update_edit(field, data_source[i]);
	}
}

EditorProperty* ArraySchema::create_property() const {
	auto prop = CommonInspectorProperty::_new();
	auto editor = ArrayEditor::_new();
	editor->_custom_init(this, prop, Array::make());
	prop->_custom_init(this, editor);
	return prop;
}

Variant StringSchema::create_value() const {
	return "";
}

Control* StringSchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	// TODO filtering support

	auto edit = LineEdit::_new();
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	edit->connect("text_changed", target, "update_value", path);
	if (name) {
		auto label = Label::_new();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = HBoxContainer::_new();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

void StringSchema::update_edit(Control* edit, const Variant& data) const {
	String text = data;
	if (auto line_edit = Object::cast_to<LineEdit>(edit)) {
		line_edit->set_text(data);
		return;
	}
	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
		auto line_edit = static_cast<LineEdit*>(editor->get_child(1));
		line_edit->set_text(text);
		return;
	}
}

EditorProperty* StringSchema::create_property() const {
	return CommonInspectorProperty::_new();
}

Variant EnumSchema::create_value() const {
	return 0;
}

Control* EnumSchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	auto edit = OptionButton::_new();
	for (auto& [name, id] : elements) {
		edit->get_popup()->add_item(name, id);
	}
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	edit->connect("item_selected", target, "update_value", path);
	if (name) {
		auto label = Label::_new();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = HBoxContainer::_new();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

void EnumSchema::update_edit(Control* edit, const Variant& data) const {
	int id = data;
	if (auto option_btn = Object::cast_to<OptionButton>(edit)) {
		option_btn->select(option_btn->get_item_index(id));
		return;
	}
	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
		auto option_btn = static_cast<OptionButton*>(editor->get_child(1));
		option_btn->select(option_btn->get_item_index(id));
		return;
	}
}

EditorProperty* EnumSchema::create_property() const {
	return CommonInspectorProperty::_new();
}

Variant IntSchema::create_value() const {
	return 0;
}

Control* IntSchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	auto edit = SpinBox::_new();
	edit->set_min(static_cast<real_t>(min_value));
	edit->set_max(static_cast<real_t>(max_value));
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	edit->connect("value_changed", target, "update_value", path);
	if (name) {
		auto label = Label::_new();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = HBoxContainer::_new();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

void IntSchema::update_edit(Control* edit, const Variant& data) const {
	int value = data;
	if (auto spin_box = Object::cast_to<SpinBox>(edit)) {
		spin_box->set_value(value);
		return;
	}
	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
		auto spin_box = static_cast<SpinBox*>(editor->get_child(1));
		spin_box->set_value(value);
		return;
	}
}

EditorProperty* IntSchema::create_property() const {
	return CommonInspectorProperty::_new();
}

Variant FloatSchema::create_value() const {
	return 0.0;
}

Control* FloatSchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	auto edit = SpinBox::_new();
	edit->set_min(min_value);
	edit->set_max(max_value);
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	edit->connect("value_changed", target, "update_value", path);
	if (name) {
		auto label = Label::_new();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = HBoxContainer::_new();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

void FloatSchema::update_edit(Control* edit, const Variant& data) const {
	real_t value = data;
	if (auto spin_box = Object::cast_to<SpinBox>(edit)) {
		spin_box->set_value(value);
		return;
	}
	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
		auto spin_box = static_cast<SpinBox*>(editor->get_child(1));
		spin_box->set_value(value);
		return;
	}
}

EditorProperty* FloatSchema::create_property() const {
	return CommonInspectorProperty::_new();
}

Variant BoolSchema::create_value() const {
	return false;
}

Control* BoolSchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	auto edit = CheckBox::_new();
	edit->connect("toggled", target, "update_value", path);
	if (name) {
		auto label = Label::_new();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = HBoxContainer::_new();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

void BoolSchema::update_edit(Control* edit, const Variant& data) const {
	bool value = data;
	if (auto checkbox = Object::cast_to<CheckBox>(edit)) {
		checkbox->set_pressed(value);
		return;
	}
	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
		auto checkbox = static_cast<CheckBox*>(editor->get_child(1));
		checkbox->set_pressed(value);
		return;
	}
}

EditorProperty* BoolSchema::create_property() const {
	return CommonInspectorProperty::_new();
}

void ResourceSchema::_register_methods() {
	register_property("properties", &ResourceSchema::properties, Array{});
}

void ResourceSchema::_init() {
}

static std::unique_ptr<Schema> parse_schema(const Dictionary& def) {
	String type = def["type"];
	if (type == "struct") {
		auto schema = std::make_unique<StructSchema>();
		Array properties = def["properties"];
		for (int i = 0; i < properties.size(); ++i) {
			Dictionary property_def = properties[i];
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
		for (int i = 0; i < properties.size(); ++i) {
			Dictionary dict = properties[i];
			if (!dict.has("name")) continue;
			info_cache.insert({ dict["name"], ::parse_schema(dict).release() });
		}
		initialized = true;
	}
	return info_cache;
}

static Dictionary save_schema(const Schema* schema) {
	Dictionary property;
	if (auto stru = dynamic_cast<const StructSchema*>(schema)) {
		Array fields;
		for (auto& [name, field] : stru->fields) {
			auto field_dict = save_schema(field.get());
			field_dict["name"] = name;
			fields.append(field_dict);
		}
		property["type"] = "struct";
		property["properties"] = fields;
	} else if (auto array = dynamic_cast<const ArraySchema*>(schema)) {
		property["type"] = "array";
		property["element_type"] = save_schema(array->element_type.get());
		property["min_elements"] = array->min_elements;
		property["max_elements"] = array->max_elements;
	} else if (auto string = dynamic_cast<const StringSchema*>(schema)) {
		property["type"] = "string";
		if (string->pattern) {
			property["pattern"] = string->pattern.value()->get_pattern();
		}
	} else if (auto en = dynamic_cast<const EnumSchema*>(schema)) {
		Array values;
		for (auto& [name, id] : en->elements) {
			Dictionary value;
			value["name"] = name;
			value["id"] = id;
		}
		property["type"] = "enum";
		property["values"] = values;
	} else if (auto sint = dynamic_cast<const IntSchema*>(schema)) {
		property["type"] = "int";
		property["min_value"] = sint->min_value;
		property["max_value"] = sint->max_value;
	} else if (auto sfloat = dynamic_cast<const FloatSchema*>(schema)) {
		property["type"] = "int";
		property["min_value"] = sfloat->min_value;
		property["max_value"] = sfloat->max_value;
	} else if (auto sbool = dynamic_cast<const BoolSchema*>(schema)) {
		property["type"] = "bool";
	}
	return property;
}

void ResourceSchema::set_info(const std::unordered_map<String, Schema*>& data) {
	properties.clear();
	for (auto& [name, field] : data) {
		properties.append(save_schema(field));
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

ResourceSchemaNode* ResourceSchemaNode::_make_child() {
	auto child = ResourceSchemaNode::_new();
	child->_custom_init(root, this);
	auto binding = Array::make(child);
	child->connect("schema_changed", this, "_child_schema_changed", binding);
	child->connect("clicked", this, "_child_clicked", binding);
	return child;
}

Schema* ResourceSchemaNode::take_schema() {
	owns_schema = false;
	return schema;
}

bool ResourceSchemaNode::_is_mouse_inside() {
	return get_global_rect().has_point(get_global_mouse_position());
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
			// Somehow type_edit doesn't call _type_selected on its own, when selecting STRUCT
			_type_selected(STRUCT);
			type_edit->select(STRUCT);

			if (parent && parent->schema_id == STRUCT) {
				::get_nc_line(field_name)->set_visible(true);
			}
		} break;

		case NOTIFICATION_RESIZED: {
			update();
		} break;
		case NOTIFICATION_DRAW: {
			static const Color colors[] = { Color::hex(0x262C3BFF), Color::hex(0x333B4FFF) };
			draw_rect(Rect2{ 4, 4, get_size().x - 4 * 2, get_size().y - 4 * 2 }, colors[mouse_inside], false, 2.0F);
		} break;
		default: {
		} break;
	}
}

void ResourceSchemaNode::_type_selected(int id) {
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
			if (owns_schema) {
				delete schema;
				schema = nullptr;
			}
		} break;
		case ARRAY: {
			free_all(list);
			list->set_visible(false);
			::get_nc_line(min_value)->set_visible(false);
			::get_nc_line(max_value)->set_visible(false);
			if (owns_schema) {
				delete schema;
				schema = nullptr;
			}
		} break;
		case STRING: {
			::get_nc_line(pattern)->set_visible(false);
			if (owns_schema) {
				delete schema;
				schema = nullptr;
			}
		} break;
		case ENUM: {
			add->set_visible(false);
			remove->set_visible(false);
			message->set_visible(false);
			free_all(list);
			list->set_visible(false);
			if (owns_schema) {
				delete schema;
				schema = nullptr;
			}
		} break;
		case INT: {
			::get_nc_line(min_value)->set_visible(false);
			::get_nc_line(max_value)->set_visible(false);
			if (owns_schema) {
				delete schema;
				schema = nullptr;
			}
		} break;
		case FLOAT: {
			::get_nc_line(min_value)->set_visible(false);
			::get_nc_line(max_value)->set_visible(false);
			if (owns_schema) {
				delete schema;
				schema = nullptr;
			}
		} break;
		case BOOL: {
			// Do nothing
		} break;
		default: {
			// Do nothing, for schema_id == UNKNOWN
		} break;
	}

	switch (id) {
		case STRUCT: {
			add->set_visible(true);
			remove->set_visible(true);
			message->set_visible(true);
			list->set_visible(true);
			schema = new StructSchema();
			schema_id = STRUCT;
			emit_signal("schema_changed");
		} break;
		case ARRAY: {
			list->set_visible(true);

			::get_nc_label(min_value)->set_text("Min elements");
			::get_nc_line(min_value)->set_visible(true);
			::get_nc_label(max_value)->set_text("Max elements");
			::get_nc_line(max_value)->set_visible(true);

			schema = new ArraySchema();
			schema_id = ARRAY;
			// This will immediately emit a `schema_changed` signal (caused by `add_child` -> ready), we need our schema setup before that
			list->add_child(_make_child());
			emit_signal("schema_changed");
		} break;
		case STRING: {
			::get_nc_line(pattern)->set_visible(true);
			schema = new StringSchema();
			schema_id = STRING;
			emit_signal("schema_changed");
		} break;
		case ENUM: {
			add->set_visible(true);
			remove->set_visible(true);
			message->set_visible(true);
			list->set_visible(true);
			schema = new EnumSchema();
			schema_id = ENUM;
			emit_signal("schema_changed");
		} break;
		case INT: {
			::get_nc_label(min_value)->set_text("Min value");
			::get_nc_line(min_value)->set_visible(true);
			::get_nc_label(max_value)->set_text("Max value");
			::get_nc_line(max_value)->set_visible(true);
			schema = new IntSchema();
			schema_id = INT;
			emit_signal("schema_changed");
		} break;
		case FLOAT: {
			::get_nc_label(min_value)->set_text("Min value");
			::get_nc_line(min_value)->set_visible(true);
			::get_nc_label(max_value)->set_text("Max value");
			::get_nc_line(max_value)->set_visible(true);
			schema = new FloatSchema();
			schema_id = FLOAT;
			emit_signal("schema_changed");
		} break;
		case BOOL: {
			schema = new BoolSchema();
			schema_id = BOOL;
			emit_signal("schema_changed");
		} break;
		default: {
			schema = nullptr;
			schema_id = UNKNOWN;
			ERR_PRINT("Unknown type id " + String::num_int64(id));
		} break;
	}

	root->emit_something_changed();
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

			auto enum_value = VBoxContainer::_new();
			list->add_child(enum_value);

			auto name_edit = LineEdit::_new();
			name_edit->connect("text_changed", this, "_enum_id_set", Array::make(enum_value));
			enum_value->add_child(::create_nc("Name", name_edit));

			auto id_edit = SpinBox::_new();
			id_edit->set_max(INT_MAX);
			id_edit->set_value(idx);
			id_edit->connect("value_changed", this, "_enum_id_set", Array::make(enum_value));
			enum_value->add_child(::create_nc("ID", id_edit));

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
	if (auto parent = Object::cast_to<ResourceSchemaNode>(get_parent()); parent && parent->schema_id == STRUCT) {
		auto parent_schema = dynamic_cast<StructSchema*>(parent->schema);
		parent_schema->fields[get_index()].name = field_name;
		root->emit_something_changed();
	}
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
	register_method("_child_clicked", &ResourceSchemaNode::_child_clicked);
	register_method("_enum_name_set", &ResourceSchemaNode::_enum_name_set);
	register_method("_enum_id_set", &ResourceSchemaNode::_enum_id_set);

	register_signal<ResourceSchemaNode>("schema_changed", Dictionary::make());
	register_signal<ResourceSchemaNode>("clicked", Dictionary::make());
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
	min_value->connect("value_changed", this, "_min_value_set");
	auto min_value_line = ::create_nc("Min value", min_value);
	min_value_line->set_visible(false);
	contents->add_child(min_value_line);

	max_value = SpinBox::_new();
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

void ResourceSchemaInspectorProperty::_prop_schema_changed(ResourceSchemaNode* node) {
	properties_schema[node->get_field_name()] = node->take_schema();
	emit_something_changed();
}

void ResourceSchemaInspectorProperty::_register_methods() {
	register_method("_toggle_editor_visibility", &ResourceSchemaInspectorProperty::_toggle_editor_visibility);
	register_method("_prop_schema_changed", &ResourceSchemaInspectorProperty::_prop_schema_changed);
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

void ResourceSchemaInspectorProperty::add_root_property(int pos) {
	// TODO update field name
	properties_schema.insert({ "", nullptr });

	auto prop = ResourceSchemaNode::_new();
	prop->_custom_init(this, nullptr);
	prop->connect("schema_changed", this, "_prop_schema_changed", Array::make(prop));
	properties->add_child(prop);
	if (pos != -1) {
		properties->move_child(prop, properties->get_child_count() - 1);
	}
	emit_something_changed();
}

void ResourceSchemaInspectorProperty::remove_root_property(int pos) {
	if (pos == -1) {
		if (selected_idx != -1) {
			properties->get_child(pos)->queue_free();
			selected_idx = -1;
		}
	} else {
		properties->get_child(pos)->queue_free();
	}
	emit_something_changed();
}

void ResourceSchemaInspectorProperty::emit_something_changed() {
	auto obj = static_cast<ResourceSchema*>(get_edited_object());
	auto prop = get_edited_property();
	obj->set_info(properties_schema);
	emit_changed(prop, obj->get(prop), "", true);
}

void ResourceSchemaInspectorProperty::update_property() {
	updating = true;
	auto obj = static_cast<ResourceSchema*>(get_edited_object());
	auto info = obj->get_info();
	for (int i = 0; i < properties->get_child_count(); ++i) {
		auto prop = static_cast<ResourceSchemaNode*>(properties->get_child(i));
		auto& schema = properties_schema[prop->get_field_name()];
		prop->update_with(schema);
	}
	updating = false;
}

ResourceSchemaInspectorProperty::ResourceSchemaInspectorProperty() {
}

ResourceSchemaInspectorProperty::~ResourceSchemaInspectorProperty() {
}
