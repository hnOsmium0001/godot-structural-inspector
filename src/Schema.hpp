#pragma once

#include <Godot.hpp>
#include <RegEx.hpp>
#include <Resource.hpp>
#include <map>
#include <vector>

namespace godot::structural_editor {

class Field {
public:
	virtual bool matches(const Variant& value) const = 0;
};

class DictionaryField : public Field {
public:
	int min_fields = 0;
	int max_fields = INT_MAX;
	RegEx name_pattern;
	std::map<String, Field*> fields;

	virtual bool matches(const Variant& value) const override;
	bool matches(const Dictionary& value) const;
};

class ArrayField : public Field {
public:
	int min_elements = 0;
	int max_elements = INT_MAX;
	RegEx name_pattern;
	std::vector<Field*> elements;

	virtual bool matches(const Variant& value) const override;
	bool matches(const Array& value) const;
};

// TODO implement
// class EnumField : public Field {
// };

class StringField : public Field {
public:
	RegEx pattern;

	virtual bool matches(const Variant& value) const override;
	bool matches(const String& value) const;
	Ref<RegExMatch> matches_pattern(const Variant& value) const;
	Ref<RegExMatch> matches_pattern(const String& value) const;
};

class IntField : public Field {
public:
	int min = INT_MIN;
	int max = INT_MAX;

	virtual bool matches(const Variant& value) const override;
	bool matches(int value) const;
};

class RealField : public Field {
public:
	bool bounded = false;
	real_t min;
	real_t max;

	virtual bool matches(const Variant& value) const override;
	bool matches(real_t value) const;
};

class BoolField : public Field {
public:
	virtual bool matches(const Variant& value) const override;
	bool matches(bool value) const;
};

class ResourceSchema : public Resource {
	GODOT_CLASS(ResourceSchema, Resource)
private:
	// type Field = Dictionary<[
	//     "type": TYPE_DICTIONARY,
	//     // When this doesn't exist, the Dictionary can hold anything
	//     optional "fields": Dictionary<[
	//         key: String, // Name of the field, regex
	//         value: Field, // Type constraint of the field
	//     ]>,
	//     optional "min_fields": int,
	//     optional "max_fields": int,
	//
	//     "type": TYPE_ARRAY,
	//     // When this doesn't exist, the Array can hold anything
	//     optional "elements": Array<Field>,
	//     optional "min_elements": int,
	//     optional "max_elements": int,
	//
	//     "type": TYPE_STRING,
	//     // When this doesn't exist, it is equivlent to `/.*/`.
	//     optional "pattern": RegEx,
	//
	//     "type": TYPE_ENUM,
	//     "values": Dictionary<[
	//          key: String, // Name of the enum value
	//          value: Int, // Integer value of the enum value
	//     ]>,
	//
	//     "type": TYPE_RESOURCE,
	//     "class": /path/to/resource/class
	//
	//     "type": TYPE_INT,
	//     optional "min_value": Int,
	//     optional "max_value": Int,
	//
	//     "type": TYPE_REAL,
	//     optional "min_value": Real,
	//     optional "max_value": Real,
	//
	//     "type": TYPE_BOOL,
	// ]>
	Array properties;

public:
	static void _register_methods();
	void _init();

	ResourceSchema();
	~ResourceSchema();
};

} // namespace godot::structural_editor
