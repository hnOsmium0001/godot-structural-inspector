#pragma once

#include "Utils.hpp"

#include <Button.hpp>
#include <Control.hpp>
#include <EditorProperty.hpp>
#include <Godot.hpp>
#include <RegEx.hpp>
#include <Resource.hpp>
#include <String.hpp>
#include <Variant.hpp>
#include <limits>
#include <memory>
#include <vector>

namespace godot::structural_inspector {

class ResourceInspectorProperty;
class Schema : public CloneProvider<Schema> {
public:
	virtual ~Schema() = default;
};

class StructSchema : public Schema, public CloneProvider<StructSchema> {
public:
	struct Field {
		String name;
		std::unique_ptr<Schema> def;
	};
	std::vector<Field> fields;

	StructSchema* clone() const override;
};

class ArraySchema : public Schema, public CloneProvider<ArraySchema> {
public:
	std::unique_ptr<Schema> element_type;
	int min_elements = 0;
	int max_elements = std::numeric_limits<int>::max();

	ArraySchema* clone() const override;
};

class StringSchema : public Schema, public CloneProvider<StringSchema> {
public:
	Ref<RegEx> pattern;

	StringSchema* clone() const override;
};

class EnumSchema : public Schema, public CloneProvider<EnumSchema> {
public:
	struct EnumValue {
		String name;
		int id;
	};
	std::vector<EnumValue> elements;

	EnumSchema* clone() const override;
};

class IntSchema : public Schema, public CloneProvider<IntSchema> {
public:
	int min_value = std::numeric_limits<int>::min();
	int max_value = std::numeric_limits<int>::max();

	IntSchema* clone() const override;
};

class FloatSchema : public Schema, public CloneProvider<FloatSchema> {
public:
	// real_t will be either float32 or float64 depending on the compilation options, so this will work
	real_t min_value = std::numeric_limits<real_t>::min();
	real_t max_value = std::numeric_limits<real_t>::max();

	FloatSchema* clone() const override;
};

class BoolSchema : public Schema, public CloneProvider<BoolSchema> {
public:
	BoolSchema* clone() const override;
};

class ResourceSchema : public Resource {
	GODOT_CLASS(ResourceSchema, Resource)
public:
	Array properties;

public:
	static void _register_methods();
	void _init();

	std::unordered_map<String, Schema*> compute_info() const;
	std::unique_ptr<Schema> compute_info_for(const String& prop_name) const;

	ResourceSchema();
	~ResourceSchema();
};

std::unique_ptr<Schema> parse_schema(const Dictionary& def);
Dictionary save_schema(Schema* schema);

} // namespace godot::structural_inspector
