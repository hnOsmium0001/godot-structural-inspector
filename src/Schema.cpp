#include "Schema.hpp"
#include "ResourceEditor.hpp"

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

StructSchema* StructSchema::clone() const {
	auto that = new StructSchema();
	for (auto& [name, field] : fields) {
		that->fields.push_back({ name, field->clone_uptr() });
	}
	return that;
}

ArraySchema* ArraySchema::clone() const {
	auto that = new ArraySchema();
	that->element_type = element_type->clone_uptr();
	that->min_elements = min_elements;
	that->max_elements = max_elements;
	return that;
}

StringSchema* StringSchema::clone() const {
	auto that = new StringSchema();
	if (pattern.is_valid()) {
		that->pattern = Ref{ RegEx::_new() };
		// Must success, since our pattern is valid
		that->pattern->compile(pattern->get_pattern());
	}
	return that;
}

EnumSchema* EnumSchema::clone() const {
	auto that = new EnumSchema();
	for (auto& element : elements) {
		that->elements.push_back(element);
	}
	return that;
}

IntSchema* IntSchema::clone() const {
	auto that = new IntSchema();
	that->min_value = min_value;
	that->max_value = max_value;
	return that;
}

FloatSchema* FloatSchema::clone() const {
	auto that = new FloatSchema();
	that->min_value = min_value;
	that->max_value = max_value;
	return that;
}

BoolSchema* BoolSchema::clone() const {
	return new BoolSchema();
}

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
