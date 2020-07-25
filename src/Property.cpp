#include "Property.hpp"

#include <CheckBox.hpp>
#include <InputEventMouseButton.hpp>
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

namespace {
std::unique_ptr<Schema> parse_properties(const Dictionary& def) {
	if (!def.has("type")) return nullptr;
	if (def["type"].get_type() != Variant::STRING) return nullptr;
	String type = def["type"];

	if (type == "struct") {
		if (!def.has("properties")) return nullptr;
		if (def["properties"].get_type() != Variant::DICTIONARY) return nullptr;
		Array properties = def["properties"];

		auto schema = std::make_unique<StructSchema>();
		for (int i = 0; i < properties.size(); ++i) {
			auto variant = properties[i];
			if (variant.get_type() != Variant::DICTIONARY) continue;
			Dictionary property_def = variant;

			if (!property_def.has("name")) continue;
			if (property_def["name"].get_type() != Variant::STRING) continue;
			String name = property_def["name"];

			schema->fields.push_back(StructSchema::Field{ name, parse_properties(property_def) });
		}

		return schema;
	} else if (type == "array") {
		if (!def.has("element_type")) return nullptr;
		if (def["element_type"].get_type() != Variant::DICTIONARY) return nullptr;
		Dictionary element_type_dict = def["element_type"];

		auto element_type = parse_properties(element_type_dict);
		if (!element_type) return nullptr;

		auto schema = std::make_unique<ArraySchema>();
		schema->element_type = std::move(element_type);
		return schema;
	} else if (type == "string") {
		if (!def.has("pattern")) return nullptr;
		if (def["pattern"].get_type() != Variant::STRING) return nullptr;
		String pattern = def["pattern"];

		auto schema = std::make_unique<StringSchema>();
		auto regex = RegEx::_new();
		if (auto err = regex->compile(pattern); err != Error::OK) {
			ERR_PRINT("Error while compiling regex pattern. Error code: " + String::num_int64(static_cast<int64_t>(err)));
			return nullptr;
		}
		schema->pattern = Ref{ regex };
		return schema;
	} else if (type == "enum") {
		if (!def.has("values")) return nullptr;
		if (def["values"].get_type() != Variant::ARRAY) return nullptr;
		Array values = def["values"];

		auto schema = std::make_unique<EnumSchema>();
		for (int i = 0; i < values.size(); ++i) {
			Dictionary value = values[i];
			if (!value.has("name")) continue;
			if (def["name"].get_type() != Variant::STRING) continue;
			String name = value["value"];
			int id = value.has("id") ? value["id"] : i;
			schema->elements.push_back({ name, id });
		}

		return schema;
	} else if (type == "int") {
		auto schema = std::make_unique<IntSchema>();
		if (def.has("min_value") && def["min_value"].get_type() == Variant::INT) {
			schema->min_value = def["min_value"];
		}
		if (def.has("max_value") && def["max_value"].get_type() == Variant::INT) {
			schema->max_value = def["max_value"];
		}
		return schema;
	} else if (type == "float") {
		auto schema = std::make_unique<FloatSchema>();
		if (def.has("min_value") && def["min_value"].get_type() == Variant::INT) {
			schema->min_value = def["min_value"];
		}
		if (def.has("max_value") && def["max_value"].get_type() == Variant::INT) {
			schema->max_value = def["max_value"];
		}
		return schema;
	} else if (type == "bool") {
		return std::make_unique<BoolSchema>();
	} else {
		return nullptr;
	}
}
} // namespace

const std::unordered_map<String, Schema*>& ResourceSchema::get_info() const {
	if (!initialized) {
		for (int i = 0; i < properties.size(); ++i) {
			Dictionary dict = properties[i];
			if (!dict.has("name")) continue;
			info_cache.insert({ dict["name"], ::parse_properties(dict).release() });
		}
		initialized = true;
	}
	return info_cache;
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
	Ref<InputEventMouseButton> mb = event;
	if (mb.is_valid() && element->get_global_rect().has_point(get_global_mouse_position())) {
		selected_idx = element->get_index();
		remove->set_disabled(false);
		return;
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

HBoxContainer* ResourceSchemaNode::_create_named_child(const String& name, Control* edit) {
	auto line = HBoxContainer::_new();
	auto label = Label::_new();
	label->set_text(name);
	label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	line->add_child(label);
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	line->add_child(edit);
	return line;
}

ResourceSchemaNode* ResourceSchemaNode::_make_child() {
	auto child = ResourceSchemaNode::_new();
	child->_custom_init(root);
	return child;
}

Schema* ResourceSchemaNode::_take_schema() {
	owns_schema = false;
	return schema;
}

void ResourceSchemaNode::_notification(int what) {
	switch (what) {
		case NOTIFICATION_RESIZED: {
			update();
		} break;
		case NOTIFICATION_DRAW: {
			draw_rect(Rect2{ -1, -1, get_size().x + 1, get_size().y + 1 }, Color::hex(0x262C3BFF), false, 2.0F);
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
			min_value_line->set_visible(false);
			max_value_line->set_visible(false);
			if (owns_schema) {
				delete schema;
				schema = nullptr;
			}
		} break;
		case STRING: {
			pattern_line->set_visible(false);
			if (owns_schema) {
				delete schema;
				schema = nullptr;
			}
		} break;
		case ENUM: {
			add->set_visible(false);
			remove->set_visible(false);
			free_all(list);
			list->set_visible(false);
			if (owns_schema) {
				delete schema;
				schema = nullptr;
			}
		} break;
		case INT: {
			min_value_line->set_visible(false);
			max_value_line->set_visible(false);
			if (owns_schema) {
				delete schema;
				schema = nullptr;
			}
		} break;
		case FLOAT: {
			min_value_line->set_visible(false);
			max_value_line->set_visible(false);
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
			list->set_visible(true);
			schema = new StructSchema();
			schema_id = STRUCT;
			emit_signal("schema_changed");
		} break;
		case ARRAY: {
			list->set_visible(true);
			list->add_child(_make_child());
			min_value_line->set_visible(true);
			max_value_line->set_visible(true);
			schema = new ArraySchema();
			schema_id = ARRAY;
			emit_signal("schema_changed");
		} break;
		case STRING: {
			pattern_line->set_visible(true);
			schema = new StringSchema();
			schema_id = STRING;
			emit_signal("schema_changed");
		} break;
		case ENUM: {
			add->set_visible(true);
			remove->set_visible(true);
			list->set_visible(true);
			schema = new EnumSchema();
			schema_id = ENUM;
			emit_signal("schema_changed");
		} break;
		case INT: {
			min_value_line->set_visible(true);
			max_value_line->set_visible(true);
			schema = new IntSchema();
			schema_id = INT;
			emit_signal("schema_changed");
		} break;
		case FLOAT: {
			min_value_line->set_visible(true);
			max_value_line->set_visible(true);
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

	selected_list_idx = -1;
	root->schema_changed();
}

void ResourceSchemaNode::_add_list_item() {
	switch (schema_id) {
		case STRUCT: {
			auto schema = dynamic_cast<StructSchema*>(this->schema);
			auto child = _make_child();
			child->connect("schema_changed", this, "_child_schema_changed", Array::make(child));
			list->add_child(child);

			root->schema_changed();
		} break;
		case ENUM: {
			auto enum_value = VBoxContainer::_new();
			list->add_child(enum_value);
			int idx = enum_value->get_index();

			auto name_edit = LineEdit::_new();
			name_edit->connect("text_changed", this, "_enum_id_set", Array::make(enum_value));
			enum_value->add_child(_create_named_child("Name", name_edit));

			auto id_edit = SpinBox::_new();
			id_edit->set_max(INT_MAX);
			id_edit->set_value(idx);
			id_edit->connect("value_changed", this, "_enum_id_set", Array::make(enum_value));
			enum_value->add_child(_create_named_child("ID", id_edit));

			auto schema = dynamic_cast<EnumSchema*>(this->schema);
			// Default ID is its index
			schema->elements.push_back({ "", idx });

			root->schema_changed();
		}
		default: {
			return;
		}
	}
}

void ResourceSchemaNode::_remove_list_item() {
	switch (schema_id) {
		case STRUCT: {
			if (selected_list_idx != -1) {
				auto schema = dynamic_cast<StructSchema*>(this->schema);
				list->get_child(selected_list_idx)->queue_free();
				schema->fields.erase(schema->fields.begin() + selected_list_idx);
				selected_list_idx = -1;

				root->schema_changed();
			}
		} break;
		case ENUM: {
			if (selected_list_idx != -1) {
				auto schema = dynamic_cast<EnumSchema*>(this->schema);
				list->get_child(selected_list_idx)->queue_free();
				schema->elements.erase(schema->elements.begin() + selected_list_idx);
				selected_list_idx = -1;

				root->schema_changed();
			}
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaNode::_min_value_set(real_t value) {
	switch (schema_id) {
		case ARRAY: {
			auto schema = dynamic_cast<ArraySchema*>(this->schema);
			schema->min_elements = static_cast<int>(value);
			root->schema_changed();
		} break;
		case INT: {
			auto schema = dynamic_cast<IntSchema*>(this->schema);
			schema->min_value = static_cast<int>(value);
			root->schema_changed();
		} break;
		case FLOAT: {
			auto schema = dynamic_cast<FloatSchema*>(this->schema);
			schema->min_value = value;
			root->schema_changed();
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
			root->schema_changed();
		} break;
		case INT: {
			auto schema = dynamic_cast<IntSchema*>(this->schema);
			schema->min_value = static_cast<int>(value);
			root->schema_changed();
		} break;
		case FLOAT: {
			auto schema = dynamic_cast<FloatSchema*>(this->schema);
			schema->min_value = value;
			root->schema_changed();
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
			root->schema_changed();
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaNode::_enum_name_set(const String& name, Control* child) {
	if (auto schema = dynamic_cast<EnumSchema*>(this->schema)) {
		schema->elements[child->get_index()].name = name;
		root->schema_changed();
	}
}

void ResourceSchemaNode::_enum_id_set(int id, Control* child) {
	if (auto schema = dynamic_cast<EnumSchema*>(this->schema)) {
		schema->elements[child->get_index()].id = id;
		root->schema_changed();
	}
}

void ResourceSchemaNode::_child_schema_changed(ResourceSchemaNode* child) {
	switch (schema_id) {
		case STRUCT: {
			auto schema = dynamic_cast<StructSchema*>(this->schema);
			schema->fields[child->get_index()].def = std::unique_ptr<Schema>(child->_take_schema());
		} break;
		case ARRAY: {
			auto schema = dynamic_cast<ArraySchema*>(this->schema);
			schema->element_type = std::unique_ptr<Schema>(child->_take_schema());
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaNode::_register_methods() {
	register_method("_notification", &ResourceSchemaNode::_notification);
	register_method("_type_selected", &ResourceSchemaNode::_type_selected);
	register_method("_add_list_item", &ResourceSchemaNode::_add_list_item);
	register_method("_remove_list_item", &ResourceSchemaNode::_remove_list_item);
	register_method("_min_value_set", &ResourceSchemaNode::_min_value_set);
	register_method("_max_value_set", &ResourceSchemaNode::_max_value_set);
	register_method("_pattern_set", &ResourceSchemaNode::_pattern_set);
	register_method("_enum_name_set", &ResourceSchemaNode::_enum_name_set);
	register_method("_enum_id_set", &ResourceSchemaNode::_enum_id_set);

	register_signal<ResourceSchemaNode>("schema_changed", Dictionary::make());
}

void ResourceSchemaNode::_init() {
	type_edit = OptionButton::_new();
	type_edit->get_popup()->add_item("Struct", STRUCT);
	type_edit->get_popup()->add_item("Array", ARRAY);
	type_edit->get_popup()->add_item("String", STRING);
	type_edit->get_popup()->add_item("Enum", ENUM);
	type_edit->get_popup()->add_item("Int", INT);
	type_edit->get_popup()->add_item("Float", FLOAT);
	type_edit->get_popup()->add_item("Bool", BOOL);
	type_edit->connect("item_selected", this, "_type_selected");
	add_child(_create_named_child("Type", type_edit));

	toolbar = HBoxContainer::_new();
	add_child(toolbar);
	add = NXButton::_new();
	add->_custom_init("Add");
	add->set_visible(false);
	add->connect("pressed", this, "_add_list_item");
	toolbar->add_child(add);
	remove = NXButton::_new();
	remove->_custom_init("Remove");
	remove->set_visible(false);
	remove->connect("pressed", this, "_remove_list_item");
	toolbar->add_child(remove);

	min_value = SpinBox::_new();
	min_value->connect("value_changed", this, "_min_value_set");
	min_value_line = _create_named_child("Min value", min_value);
	min_value_line->set_visible(false);
	add_child(min_value_line);

	max_value = SpinBox::_new();
	max_value->connect("value_changed", this, "_max_value_set");
	max_value_line = _create_named_child("Max value", max_value);
	max_value_line->set_visible(false);
	add_child(max_value_line);

	pattern = LineEdit::_new();
	pattern->connect("text_changed", this, "_pattern_set");
	pattern_line = _create_named_child("Pattern", pattern);
	pattern_line->set_visible(false);
	add_child(pattern_line);

	list = VBoxContainer::_new();
	list->set_visible(false);
	auto padding_box = MarginContainer::_new();
	padding_box->add_constant_override("margin_right", 0);
	padding_box->add_constant_override("margin_top", 0);
	padding_box->add_constant_override("margin_left", 16);
	padding_box->add_constant_override("margin_bottom", 0);
	padding_box->add_child(list);
	add_child(padding_box);

	// Default to struct
	type_edit->select(STRUCT);
}

void ResourceSchemaNode::_custom_init(ResourceSchemaInspectorProperty* root) {
	this->root = root;
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

void ResourceSchemaInspectorProperty::_register_methods() {
	register_method("_toggle_editor_visibility", &ResourceSchemaInspectorProperty::_toggle_editor_visibility);
	register_method("add_root_property", &ResourceSchemaInspectorProperty::add_root_property);
	register_method("remove_root_property", &ResourceSchemaInspectorProperty::remove_root_property);
	register_method("schema_chagned", &ResourceSchemaInspectorProperty::schema_changed);
	register_method("update_property", &ResourceSchemaInspectorProperty::update_property);
}

std::unique_ptr<Schema> ResourceSchemaInspectorProperty::build() {
	return nullptr; // TODO
}

void ResourceSchemaInspectorProperty::_init() {
	btn = Button::_new();
	btn->set_text("Properties");
	btn->connect("pressed", this, "_toggle_editor_visibility");
	add_child(btn);

	properties = VBoxContainer::_new();
	properties->set_visible(false);

	auto toolbar = HBoxContainer::_new();
	auto add = Button::_new();
	add->set_flat(true);
	add->set_button_icon(get_icon("Add", "EditorIcons"));
	add->connect("pressed", this, "add_root_property");
	toolbar->add_child(add);
	auto remove = Button::_new();
	remove->set_flat(true);
	remove->set_button_icon(get_icon("Remove", "EditorIcons"));
	remove->connect("pressed", this, "remove_root_property");
	toolbar->add_child(remove);

	properties->add_child(toolbar);
}

void ResourceSchemaInspectorProperty::add_root_property(int pos) {
	auto prop = ResourceSchemaNode::_new();
	prop->_custom_init(this);
	prop->connect("schema_changed", this, "schema_changed", Array::make(prop));
	properties->add_child(prop);
	if (pos != -1) {
		properties->move_child(prop, properties->get_child_count() - 1);
	}
	schema_changed();
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
	schema_changed();
}

void ResourceSchemaInspectorProperty::schema_changed() {
	// TODO save data
	emit_changed(get_edited_property(), get_edited_object()->get(get_edited_property()), "", true);
}

void ResourceSchemaInspectorProperty::update_property() {
	updating = true;
	Array root = get_edited_object()->get(get_edited_property());
	// TODO
	updating = false;
}

ResourceSchemaInspectorProperty::ResourceSchemaInspectorProperty() {
}

ResourceSchemaInspectorProperty::~ResourceSchemaInspectorProperty() {
}
