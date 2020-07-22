#include "Schema.hpp"

using namespace godot;
using namespace godot::structural_editor;

bool DictionaryField::matches(const Variant& value) const {
	if (value.get_type() != Variant::DICTIONARY) {
		return false;
	} else {
		Dictionary dict = value;
		return matches(dict);
	}
}

bool DictionaryField::matches(const Dictionary& value) const {
	return false; // TODO
}

bool ArrayField::matches(const Variant& value) const {
	if (value.get_type() != Variant::ARRAY) {
		return false;
	} else {
		Array arr = value;
		return matches(arr);
	}
}

bool ArrayField::matches(const Array& value) const {
	return false; // TODO
}

bool StringField::matches(const Variant& value) const {
	auto result = matches_pattern(value);
	return result.is_valid();
}

bool StringField::matches(const String& value) const {
	auto result = matches_pattern(value);
	return result.is_valid();
}

Ref<RegExMatch> StringField::matches_pattern(const Variant& value) const {
	if (value.get_type() != Variant::STRING) {
		return Ref<RegExMatch>{ nullptr };
	} else {
		String str = value;
		return matches_pattern(str);
	}
}

Ref<RegExMatch> StringField::matches_pattern(const String& value) const {
	return pattern.search(value);
}

bool IntField::matches(const Variant& value) const {
	if (value.get_type() != Variant::INT) {
		return false;
	} else {
		int i = value;
		return matches(i);
	}
}

bool IntField::matches(int value) const {
	return value >= min && value <= max;
}

bool RealField::matches(const Variant& value) const {
	if (!bounded) {
		return true;
	}
	if (value.get_type() != Variant::REAL) {
		return false;
	} else {
		real_t r = value;
		return r >= min && r <= max;
	}
}

bool RealField::matches(real_t value) const {
	return !bounded || value >= min && value <= max;
}

bool BoolField::matches(const Variant& value) const {
	return value.get_type() != Variant::BOOL;
}

bool BoolField::matches(bool value) const {
	return true;
}

void ResourceSchema::_register_methods() {
	register_property("properties", &ResourceSchema::properties, Array{}, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_EDITOR);
}

void ResourceSchema::_init() {
}

ResourceSchema::ResourceSchema() {
}

ResourceSchema::~ResourceSchema() {
}
