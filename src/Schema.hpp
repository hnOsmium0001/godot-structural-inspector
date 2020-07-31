#pragma once

#include "Utils.hpp"

#include <Button.hpp>
#include <Control.hpp>
#include <EditorProperty.hpp>
#include <Godot.hpp>
#include <RegEx.hpp>
#include <String.hpp>
#include <Variant.hpp>
#include <limits>
#include <memory>
#include <vector>

namespace godot::structural_inspector {

class CommonInspectorProperty;

class Schema : public CloneProvider<Schema> {
public:
	virtual Variant create_value() const = 0;
	virtual Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const = 0;
	virtual void update_edit(Control* edit, const Variant& data) const = 0;
	virtual EditorProperty* create_property() const = 0;
	virtual ~Schema() = default;
};

struct NamedSchema {
	String name;
	std::unique_ptr<Schema> def;
};

class StructSchema : public Schema, public CloneProvider<StructSchema> {
public:
	std::vector<NamedSchema> fields;

	Variant create_value() const override;
	Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	void update_edit(Control* edit, const Variant& data) const override;
	EditorProperty* create_property() const override;
	StructSchema* clone() const override;

	static StructSchema defaulted();
};

class ArraySchema : public Schema, public CloneProvider<ArraySchema> {
public:
	std::unique_ptr<Schema> element_type;
	int min_elements = std::numeric_limits<int>::min();
	int max_elements = std::numeric_limits<int>::max();

	Variant create_value() const override;
	Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	void update_edit(Control* edit, const Variant& data) const override;
	EditorProperty* create_property() const override;
	ArraySchema* clone() const override;

	static ArraySchema defaulted();
};

class StringSchema : public Schema, public CloneProvider<StringSchema> {
public:
	Ref<RegEx> pattern;

	Variant create_value() const override;
	Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	void update_edit(Control* edit, const Variant& data) const override;
	EditorProperty* create_property() const override;
	StringSchema* clone() const override;

	static StructSchema defaulted();
};

class EnumSchema : public Schema, public CloneProvider<EnumSchema> {
public:
	struct EnumValue {
		String name;
		int id;
	};
	std::vector<EnumValue> elements;

	Variant create_value() const override;
	Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	void update_edit(Control* edit, const Variant& data) const override;
	EditorProperty* create_property() const override;
	EnumSchema* clone() const override;

	static EnumSchema defaulted();
};

class IntSchema : public Schema, public CloneProvider<IntSchema> {
public:
	int min_value = std::numeric_limits<int>::min();
	int max_value = std::numeric_limits<int>::max();

	Variant create_value() const override;
	Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	void update_edit(Control* edit, const Variant& data) const override;
	EditorProperty* create_property() const override;
	IntSchema* clone() const override;

	static IntSchema defaulted();
};

class FloatSchema : public Schema, public CloneProvider<FloatSchema> {
public:
	// real_t will be either float32 or float64 depending on the compilation options, so this will work
	real_t min_value = std::numeric_limits<real_t>::min();
	real_t max_value = std::numeric_limits<real_t>::max();

	Variant create_value() const override;
	Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	void update_edit(Control* edit, const Variant& data) const override;
	EditorProperty* create_property() const override;
	FloatSchema* clone() const override;

	static FloatSchema defaulted();
};

class BoolSchema : public Schema, public CloneProvider<BoolSchema> {
public:
	Variant create_value() const override;
	Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	void update_edit(Control* edit, const Variant& data) const override;
	EditorProperty* create_property() const override;
	BoolSchema* clone() const override;

	static BoolSchema defaulted();
};

} // namespace godot::structural_inspector
