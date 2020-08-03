#include "Schema.hpp"
#include "ResourceEditor.hpp"
#include "Variant.hpp"

#include <CheckBox.hpp>
#include <Control.hpp>
#include <HBoxContainer.hpp>
#include <JSON.hpp>
#include <Label.hpp>
#include <LineEdit.hpp>
#include <OptionButton.hpp>
#include <PopupMenu.hpp>
#include <SpinBox.hpp>

using namespace godot;
using namespace godot::structural_inspector;

void ResourceSchema::_register_methods() {
	register_property("properties", &ResourceSchema::properties, Array{});
}

void ResourceSchema::_init() {
}

std::unordered_map<String, Schema*> ResourceSchema::compute_info() const {
	std::unordered_map<String, Schema*> info;
	for (int i = 0; i < properties.size(); ++i) {
		Dictionary dict = properties[i];
		// If parsing schema failed, we skip this entry
		if (auto schema = ::parse_schema(dict)) {
			info.insert({ dict["name"], schema.release() });
		} else {
			ERR_PRINT("Error while parsing schema entry: " + JSON::get_singleton()->print(dict));
		}
	}
	return info;
}

std::unique_ptr<Schema> ResourceSchema::compute_info_for(const String& prop_name) const {
	for (int i = 0; i < properties.size(); ++i) {
		Dictionary dict = properties[i];
		if (dict.has("name") && dict["name"].get_type() == Variant::STRING) {
			String name = dict["name"];
			if (name == prop_name) {
				return ::parse_schema(dict);
			}
		}
	}
	return nullptr;
}

ResourceSchema::ResourceSchema() {
}

ResourceSchema::~ResourceSchema() {
}

// void Schema::update_value(ResourceInspectorProperty* root, const Variant& value) const {
// 	root->push_key(key);
// 	if (parent) {
// 		parent->update_value(root, value);
// 	} else {
// 		root->set_current_value(value);
// 	}
// }

// Variant StructSchema::create_value() const {
// 	return Dictionary{};
// }

// Control* StructSchema::create_edit(const String* name, ResourceInspectorProperty* root) const {
// 	auto editor = StructEditor::_new();
// 	editor->_custom_init(this, root, key);
// 	if (name) {
// 		editor->set_property_name(*name);
// 	}
// 	return editor;
// }

// void StructSchema::update_edit(Control* edit, const Variant& data) const {
// 	auto editor = static_cast<StructEditor*>(edit);
// 	Dictionary data_source = data;
// 	for (int i = 0; i < fields.size(); ++i) {
// 		auto& field = fields[i];
// 		field.def->update_edit(
// 				static_cast<Control*>(editor->fields->get_child(i)),
// 				data_source[field.name]);
// 	}
// }

StructSchema* StructSchema::clone() const {
	auto that = new StructSchema();
	for (auto& [name, field] : fields) {
		that->fields.push_back({ name, field->clone_uptr() });
	}
	return that;
}

// Variant ArraySchema::create_value() const {
// 	return Array{};
// }

// Control* ArraySchema::create_edit(const String* name, ResourceInspectorProperty* root) const {
// 	auto editor = ArrayEditor::_new();
// 	editor->_custom_init(this, root, key);
// 	if (name) {
// 		editor->set_property_name(*name);
// 	}
// 	return editor;
// }

// void ArraySchema::update_edit(Control* edit, const Variant& data) const {
// 	auto editor = static_cast<ArrayEditor*>(data);
// 	Array data_source = data;
// 	for (int i = 0; i < data_source.size(); ++i) {
// 		auto field = static_cast<Control*>(editor->elements->get_child(i));
// 		element_type->update_edit(field, data_source[i]);
// 	}
// }

ArraySchema* ArraySchema::clone() const {
	auto that = new ArraySchema();
	that->element_type = element_type->clone_uptr();
	that->min_elements = min_elements;
	that->max_elements = max_elements;
	return that;
}

// Variant StringSchema::create_value() const {
// 	return "";
// }

// Control* StringSchema::create_edit(const String* name, ResourceInspectorProperty* root) const {
// 	// TODO filtering support

// 	auto edit = LineEdit::_new();
// 	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
// 	edit->connect("text_changed", root, "update_value", path);
// 	if (name) {
// 		auto label = Label::_new();
// 		label->set_text(*name);
// 		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

// 		auto box = HBoxContainer::_new();
// 		box->add_child(label);
// 		box->add_child(edit);
// 		return box;
// 	} else {
// 		return edit;
// 	}
// }

// void StringSchema::update_edit(Control* edit, const Variant& data) const {
// 	String text = data;
// 	if (auto line_edit = Object::cast_to<LineEdit>(edit)) {
// 		line_edit->set_text(data);
// 		return;
// 	}
// 	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
// 		auto line_edit = static_cast<LineEdit*>(editor->get_child(1));
// 		line_edit->set_text(text);
// 		return;
// 	}
// }

StringSchema* StringSchema::clone() const {
	auto that = new StringSchema();
	if (pattern.is_valid()) {
		that->pattern = Ref{ RegEx::_new() };
		// Must success, since our pattern is valid
		that->pattern->compile(pattern->get_pattern());
	}
	return that;
}

// Variant EnumSchema::create_value() const {
// 	return 0;
// }

// Control* EnumSchema::create_edit(const String* name, ResourceInspectorProperty* root) const {
// 	auto edit = ValueEditor::_new();
// 	edit->_custom_init(this, name);
// 	return edit;
// }

// void EnumSchema::update_edit(Control* edit, const Variant& data) const {
// 	int id = data;
// 	if (auto option_btn = Object::cast_to<OptionButton>(edit)) {
// 		option_btn->select(option_btn->get_item_index(id));
// 		return;
// 	}
// 	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
// 		auto option_btn = static_cast<OptionButton*>(editor->get_child(1));
// 		option_btn->select(option_btn->get_item_index(id));
// 		return;
// 	}
// }

EnumSchema* EnumSchema::clone() const {
	auto that = new EnumSchema();
	for (auto& element : elements) {
		that->elements.push_back(element);
	}
	return that;
}

// Variant IntSchema::create_value() const {
// 	return 0;
// }

// Control* IntSchema::create_edit(const String* name, ResourceInspectorProperty* root) const {
// 	auto edit = ValueEditor::_new();
// 	edit->_custom_init(this, name);
// 	return edit;
// }

// void IntSchema::update_edit(Control* edit, const Variant& data) const {
// 	int value = data;
// 	if (auto spin_box = Object::cast_to<SpinBox>(edit)) {
// 		spin_box->set_value(value);
// 		return;
// 	}
// 	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
// 		auto spin_box = static_cast<SpinBox*>(editor->get_child(1));
// 		spin_box->set_value(value);
// 		return;
// 	}
// }

IntSchema* IntSchema::clone() const {
	auto that = new IntSchema();
	that->min_value = min_value;
	that->max_value = max_value;
	return that;
}

// Variant FloatSchema::create_value() const {
// 	return 0.0;
// }

// Control* FloatSchema::create_edit(const String* name, ResourceInspectorProperty* root) const {
// 	auto edit = ValueEditor::_new();
// 	edit->_custom_init(this, name);
// 	return edit;
// }

// void FloatSchema::update_edit(Control* edit, const Variant& data) const {
// 	real_t value = data;
// 	if (auto spin_box = Object::cast_to<SpinBox>(edit)) {
// 		spin_box->set_value(value);
// 		return;
// 	}
// 	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
// 		auto spin_box = static_cast<SpinBox*>(editor->get_child(1));
// 		spin_box->set_value(value);
// 		return;
// 	}
// }

FloatSchema* FloatSchema::clone() const {
	auto that = new FloatSchema();
	that->min_value = min_value;
	that->max_value = max_value;
	return that;
}

// Variant BoolSchema::create_value() const {
// 	return false;
// }

// Control* BoolSchema::create_edit(const String* name, ResourceInspectorProperty* root) const {
// 	auto edit = ValueEditor::_new();
// 	edit->_custom_init(this, name);
// 	return edit;
// }

// void BoolSchema::update_edit(Control* edit, const Variant& data) const {
// 	bool value = data;
// 	if (auto checkbox = Object::cast_to<CheckBox>(edit)) {
// 		checkbox->set_pressed(value);
// 		return;
// 	}
// 	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
// 		auto checkbox = static_cast<CheckBox*>(editor->get_child(1));
// 		checkbox->set_pressed(value);
// 		return;
// 	}
// }

BoolSchema* BoolSchema::clone() const {
	return new BoolSchema();
}

std::unique_ptr<Schema> godot::structural_inspector::parse_schema(const Dictionary& def) {
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

Dictionary godot::structural_inspector::save_schema(Schema* schema) {
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
			values.append(value);
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
