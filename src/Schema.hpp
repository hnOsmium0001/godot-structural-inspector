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

class ResourceInspectorProperty;
class Schema : public CloneProvider<Schema> {
public:
	// virtual Variant create_value() const = 0;
	// virtual Control* create_edit(const String* name, ResourceInspectorProperty* root) const = 0;
	// virtual void update_edit(Control* edit, const Variant& data) const = 0;
	// virtual void update_value(ResourceInspectorProperty* root, const Variant& value) const;

	virtual ~Schema() = default;
};

class StructSchema : public Schema, public CloneProvider<StructSchema> {
public:
	struct Field {
		String name;
		std::unique_ptr<Schema> def;
	};
	std::vector<Field> fields;

	// Variant create_value() const override;
	// Control* create_edit(const String* name, ResourceInspectorProperty* root) const override;
	// void update_edit(Control* edit, const Variant& data) const override;

	StructSchema* clone() const override;
};

class ArraySchema : public Schema, public CloneProvider<ArraySchema> {
public:
	std::unique_ptr<Schema> element_type;
	int min_elements = std::numeric_limits<int>::min();
	int max_elements = std::numeric_limits<int>::max();

	// Variant create_value() const override;
	// Control* create_edit(const String* name, ResourceInspectorProperty* root) const override;
	// void update_edit(Control* edit, const Variant& data) const override;

	ArraySchema* clone() const override;
};

class StringSchema : public Schema, public CloneProvider<StringSchema> {
public:
	Ref<RegEx> pattern;

	// Variant create_value() const override;
	// Control* create_edit(const String* name, ResourceInspectorProperty* root) const override;
	// void update_edit(Control* edit, const Variant& data) const override;

	StringSchema* clone() const override;
};

class EnumSchema : public Schema, public CloneProvider<EnumSchema> {
public:
	struct EnumValue {
		String name;
		int id;
	};
	std::vector<EnumValue> elements;

	// Variant create_value() const override;
	// Control* create_edit(const String* name, ResourceInspectorProperty* root) const override;
	// void update_edit(Control* edit, const Variant& data) const override;

	EnumSchema* clone() const override;
};

class IntSchema : public Schema, public CloneProvider<IntSchema> {
public:
	int min_value = std::numeric_limits<int>::min();
	int max_value = std::numeric_limits<int>::max();

	// Variant create_value() const override;
	// Control* create_edit(const String* name, ResourceInspectorProperty* root) const override;
	// void update_edit(Control* edit, const Variant& data) const override;
	
	IntSchema* clone() const override;
};

class FloatSchema : public Schema, public CloneProvider<FloatSchema> {
public:
	// real_t will be either float32 or float64 depending on the compilation options, so this will work
	real_t min_value = std::numeric_limits<real_t>::min();
	real_t max_value = std::numeric_limits<real_t>::max();

	// Variant create_value() const override;
	// Control* create_edit(const String* name, ResourceInspectorProperty* root) const override;
	// void update_edit(Control* edit, const Variant& data) const override;

	FloatSchema* clone() const override;
};

class BoolSchema : public Schema, public CloneProvider<BoolSchema> {
public:
	// Variant create_value() const override;
	// Control* create_edit(const String* name, ResourceInspectorProperty* root) const override;
	// void update_edit(Control* edit, const Variant& data) const override;

	BoolSchema* clone() const override;
};

std::unique_ptr<Schema> parse_schema(const Dictionary& def);
Dictionary save_schema(Schema* schema);

} // namespace godot::structural_inspector
