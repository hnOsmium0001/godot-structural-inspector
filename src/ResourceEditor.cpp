#include "ResourceEditor.hpp"

#include <CheckBox.hpp>
#include <InputEvent.hpp>
#include <InputEventMouseButton.hpp>
#include <JSON.hpp>
#include <PopupMenu.hpp>
#include <utility>

using namespace godot;
using namespace godot::structural_inspector;

void ResourceEditor::_register_methods() {
}

void ResourceEditor::_init() {
}

void ResourceEditor::set_key(const Variant& key) {
}

void ResourceEditor::read(const Variant& value) {
}

Variant ResourceEditor::save() const {
	return Variant{};
}

ResourceEditor::ResourceEditor() {
}

ResourceEditor::~ResourceEditor() {
}

static std::pair<Control*, ResourceEditor*> create_edit_overloaded(
		ResourceInspectorProperty* root,
		ResourceEditor* parent,
		const Schema* schema,
		const Variant& key,
		bool add_border = false) {
	if (auto sch = dynamic_cast<const StructSchema*>(schema)) {
		auto edit = StructEditor::_new();
		edit->_custom_init(root, parent, sch, key);

		if (add_border) {
			auto container = BorderedContainer::_new();
			container->add_child(edit);
			return { container, edit };
		} else {
			return { edit, edit };
		}
	} else if (auto sch = dynamic_cast<const ArraySchema*>(schema)) {
		auto edit = ArrayEditor::_new();
		edit->_custom_init(root, parent, sch, key);

		if (add_border) {
			auto container = BorderedContainer::_new();
			container->add_child(edit);
			return { container, edit };
		} else {
			return { edit, edit };
		}
	} else {
		auto edit = ValueEditor::_new();
		edit->_custom_init(root, parent, schema, key);
		return { edit, edit };
	}
}

static ResourceEditor* find_editor_from(Node* c) {
	if (auto container = Object::cast_to<MarginContainer>(c)) {
		if (container->get_child_count() > 0) {
			return Object::cast_to<ResourceEditor>(container->get_child(0));
		} else {
			return nullptr;
		}
	} else {
		return Object::cast_to<ResourceEditor>(c);
	}
}

static void format_key_to(const Variant& key, Label* label) {
	switch (key.get_type()) {
		case Variant::STRING: {
			label->set_visible(true);
			label->set_text(key);
		} break;
		case Variant::INT: {
			label->set_visible(true);
			label->set_text("[" + String::num_int64(key) + "]");
		} break;
		default: {
			label->set_visible(false);
		} break;
	}
}

void StructEditor::_notification(int what) {
	ListContainer::_notification(what);
}

Size2 StructEditor::_get_minimum_size() {
	return ListContainer::_get_minimum_size();
}

void StructEditor::_register_methods() {
	register_method("_notification", &StructEditor::_notification);
	register_method("_get_minimum_size", &StructEditor::_get_minimum_size);
}

void StructEditor::_init() {
	set_direction(VERTICAL);

	toolbar = HBoxContainer::_new();
	add_child(toolbar);

	title = Label::_new();
	toolbar->add_child(title);

	fields = VBoxContainer::_new();
	add_child(fields);
}

void StructEditor::_custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const StructSchema* schema, const Variant& key) {
	this->root = root;
	this->parent = parent;
	this->schema = schema;

	format_key_to(key, title);

	for (auto& [name, field] : schema->fields) {
		fields->add_child(create_edit_overloaded(root, this, field.get(), name).first);
	}
}

void StructEditor::set_key(const Variant& key) {
	format_key_to(key, title);
}

void StructEditor::read(const Variant& value) {
	if (value.get_type() != Variant::DICTIONARY) {
		return;
	}

	Dictionary dict = value;
	int i = 0;
	for (auto& [name, _] : schema->fields) {
		Object::cast_to<ResourceEditor>(fields->get_child(i))->read(dict[name]);
		++i;
	}
}

Variant StructEditor::save() const {
	Dictionary dict;
	int i = 0;
	for (auto& [name, _] : schema->fields) {
		auto field = find_editor_from(fields->get_child(i));
		dict[name] = field->save();
		++i;
	}
	return dict;
}

StructEditor::StructEditor() {
}

StructEditor::~StructEditor() {
}

ResourceEditor* ArrayEditor::_get_editor_at(int idx) {
	if (auto container = Object::cast_to<MarginContainer>(elements->get_child(idx))) {
		return reinterpret_cast<ResourceEditor*>(container->get_child(0));
	} else {
		return nullptr;
	}
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
	int idx = elements->get_child_count();
	auto element = create_edit_overloaded(root, this, schema->element_type.get(), idx, true).first;
	element->connect("gui_input", this, "_element_gui_input", Array::make(element));

	elements->add_child(element);
	if (selected_idx != -1 && selected_idx < elements->get_child_count() - 1) {
		elements->move_child(element, selected_idx + 1);
		for (int i = selected_idx + 2; i < elements->get_child_count(); ++i) {
			_get_editor_at(i)->set_key(i);
		}
	}

	if (elements->get_child_count() == schema->max_elements) {
		add->set_disabled(true);
	}

	root->emit_something_changed();
}

void ArrayEditor::_remove_element() {
	if (selected_idx != -1) {
		elements->get_child(selected_idx)->free();
		// The elements after the freed element was moved forward, as we used free() instead of queue_free()
		for (int i = selected_idx; i < elements->get_child_count(); ++i) {
			_get_editor_at(i)->set_key(i - 1);
		}

		selected_idx = -1;
		remove->set_disabled(true);
		if (elements->get_child_count() < schema->max_elements) {
			add->set_disabled(false);
		}

		root->emit_something_changed();
	}
}

void ArrayEditor::_notification(int what) {
	ListContainer::_notification(what);
}

Size2 ArrayEditor::_get_minimum_size() {
	return ListContainer::_get_minimum_size();
}

void ArrayEditor::_register_methods() {
	register_method("_notification", &ArrayEditor::_notification);
	register_method("_get_minimum_size", &ArrayEditor::_get_minimum_size);
	register_method("_element_gui_input", &ArrayEditor::_element_gui_input);
	register_method("_add_element", &ArrayEditor::_add_element);
	register_method("_remove_element", &ArrayEditor::_remove_element);
}

void ArrayEditor::_init() {
	set_direction(VERTICAL);

	toolbar = HBoxContainer::_new();
	add_child(toolbar);

	title = Label::_new();
	toolbar->add_child(title);
	add = EditorIconButton::_new();
	add->_custom_init("Add");
	add->connect("pressed", this, "_add_element");
	toolbar->add_child(add);
	remove = EditorIconButton::_new();
	remove->_custom_init("Remove");
	remove->connect("pressed", this, "_remove_element");
	toolbar->add_child(remove);

	elements = VBoxContainer::_new();
	add_child(elements);
}

void ArrayEditor::_custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const ArraySchema* schema, const Variant& key) {
	this->root = root;
	this->parent = parent;
	this->schema = schema;

	format_key_to(key, title);

	for (int i = 0; i < schema->min_elements; ++i) {
		elements->add_child(create_edit_overloaded(root, this, schema->element_type.get(), i).first);
	}
}

void ArrayEditor::set_key(const Variant& key) {
	format_key_to(key, title);
}

void ArrayEditor::read(const Variant& value) {
	if (value.get_type() != Variant::ARRAY) {
		return;
	}

	Array array = value;
	for (int i = 0; i < elements->get_child_count(); ++i) {
		elements->get_child(i)->free();
	}
	for (int i = 0; i < array.size(); ++i) {
		auto [container, editor] = create_edit_overloaded(root, this, schema->element_type.get(), i, true);
		elements->add_child(container);
		editor->read(array[i]);
	}
}

Variant ArrayEditor::save() const {
	Array array;
	for (int i = 0; i < elements->get_child_count(); ++i) {
		auto field = find_editor_from(elements->get_child(i));
		array.append(field->save());
	}
	return array;
}

ArrayEditor::ArrayEditor() {
}

ArrayEditor::~ArrayEditor() {
}

void ValueEditor::_notification(int what) {
	ListContainer::_notification(what);
}

Size2 ValueEditor::_get_minimum_size() {
	return ListContainer::_get_minimum_size();
}

void ValueEditor::_update_string_value(const String& value) {
	root->emit_something_changed();
}

void ValueEditor::_update_enum_value(int idx) {
	root->emit_something_changed();
}

void ValueEditor::_update_int_value(int value) {
	root->emit_something_changed();
}

void ValueEditor::_update_float_value(float value) {
	root->emit_something_changed();
}

void ValueEditor::_update_bool_value(bool value) {
	root->emit_something_changed();
}

void ValueEditor::_register_methods() {
	register_method("_notification", &ValueEditor::_notification);
	register_method("_get_minimum_size", &ValueEditor::_get_minimum_size);
	register_method("_update_string_value", &ValueEditor::_update_string_value);
	register_method("_update_enum_value", &ValueEditor::_update_enum_value);
	register_method("_update_int_value", &ValueEditor::_update_int_value);
	register_method("_update_float_value", &ValueEditor::_update_float_value);
	register_method("_update_bool_value", &ValueEditor::_update_bool_value);
}

void ValueEditor::_init() {
	set_direction(HORIZONTAL);
	set_stretching(true);
}

void ValueEditor::_custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const Schema* schema, const Variant& key) {
	this->root = root;
	this->parent = parent;
	this->schema = schema;

	if (key.get_type() == Variant::STRING) {
		title = Label::_new();
		title->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
		title->set_text(key);
		add_child(title);
	}

	if (auto sch = dynamic_cast<const StringSchema*>(schema)) {
		// TODO pattern filtering
		auto edit = LineEdit::_new();
		this->edit = edit;
		edit->connect("text_changed", this, "_update_string_value");
	} else if (auto sch = dynamic_cast<const EnumSchema*>(schema)) {
		auto edit = OptionButton::_new();
		this->edit = edit;
		edit->connect("item_selected", this, "_update_enum_value");
		for (auto& [name, id] : sch->elements) {
			edit->get_popup()->add_item(name, id);
		}
	} else if (auto sch = dynamic_cast<const IntSchema*>(schema)) {
		auto edit = SpinBox::_new();
		this->edit = edit;
		edit->set_min(sch->min_value);
		edit->set_max(sch->max_value);
		edit->connect("value_changed", this, "_update_int_value");
	} else if (auto sch = dynamic_cast<const FloatSchema*>(schema)) {
		auto edit = SpinBox::_new();
		this->edit = edit;
		edit->set_min(sch->min_value);
		edit->set_max(sch->max_value);
		edit->connect("value_changed", this, "_update_float_value");
	} else if (auto sch = dynamic_cast<const BoolSchema*>(schema)) {
		auto edit = CheckBox::_new();
		this->edit = edit;
		edit->connect("toggled", this, "_update_bool_value");
	} else {
		auto edit = Label::_new();
		this->edit = edit;
		edit->set_text("Unknown schema type. This is a bug, please report immediately.");
		ERR_PRINT("Unknown schema type for ValueEditor.");
	}
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	add_child(edit);
}

void ValueEditor::set_key(const Variant& key) {
	if (title) {
		format_key_to(key, title);
	}
}

void ValueEditor::read(const Variant& value) {
	if (auto sch = dynamic_cast<const StringSchema*>(schema) && value.get_type() == Variant::STRING) {
		Object::cast_to<LineEdit>(edit)->set_text(value);
	} else if (auto sch = dynamic_cast<const EnumSchema*>(schema) && value.get_type() == Variant::INT) {
		Object::cast_to<OptionButton>(edit)->select(value);
	} else if (auto sch = dynamic_cast<const IntSchema*>(schema) && value.get_type() == Variant::INT) {
		Object::cast_to<SpinBox>(edit)->set_value(value);
	} else if (auto sch = dynamic_cast<const FloatSchema*>(schema) && value.get_type() == Variant::REAL) {
		Object::cast_to<SpinBox>(edit)->set_value(value);
	} else if (auto sch = dynamic_cast<const BoolSchema*>(schema) && value.get_type() == Variant::BOOL) {
		Object::cast_to<OptionButton>(edit)->select(value ? 1 : 0);
	}
}

Variant ValueEditor::save() const {
	if (auto sch = dynamic_cast<const StringSchema*>(schema)) {
		return Object::cast_to<LineEdit>(edit)->get_text();
	} else if (auto sch = dynamic_cast<const EnumSchema*>(schema)) {
		return Object::cast_to<OptionButton>(edit)->get_selected_id();
	} else if (auto sch = dynamic_cast<const IntSchema*>(schema)) {
		return static_cast<int>(Object::cast_to<SpinBox>(edit)->get_value());
	} else if (auto sch = dynamic_cast<const FloatSchema*>(schema)) {
		return Object::cast_to<SpinBox>(edit)->get_value();
	} else if (auto sch = dynamic_cast<const BoolSchema*>(schema)) {
		return Object::cast_to<OptionButton>(edit)->is_pressed();
	} else {
		return Variant{};
	}
}

ValueEditor::ValueEditor() {
}

ValueEditor::~ValueEditor() {
}

void ResourceInspectorProperty::_toggle_editor_visibility() {
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

void ResourceInspectorProperty::_register_methods() {
	register_method("_toggle_editor_visibility", &ResourceInspectorProperty::_toggle_editor_visibility);
	register_method("emit_something_changed", &ResourceInspectorProperty::emit_something_changed);
	register_method("update_property", &ResourceInspectorProperty::update_property);
}

void ResourceInspectorProperty::_init() {
	btn = Button::_new();
	btn->connect("pressed", this, "_toggle_editor_visibility");
	add_child(btn);
}

void ResourceInspectorProperty::_custom_init(std::unique_ptr<Schema> schema_in) {
	this->schema = std::move(schema_in);
	auto schema = this->schema.get();

	editor = create_edit_overloaded(this, nullptr, schema, Variant{}).second;
	editor->set_visible(false);
}

void ResourceInspectorProperty::emit_something_changed() {
	if (updating) return;

	emit_changed(get_edited_property(), editor->save(), "", true);
}

void ResourceInspectorProperty::update_property() {
	updating = true;

	auto prop = get_edited_object()->get(get_edited_property());
	editor->read(prop);

	updating = false;
}

ResourceInspectorProperty::ResourceInspectorProperty() {
}

ResourceInspectorProperty::~ResourceInspectorProperty() {
}
