#include "ResourceEditor.hpp"

#include <CheckBox.hpp>
#include <InputEvent.hpp>
#include <InputEventMouseButton.hpp>
#include <PopupMenu.hpp>
#include <utility>

using namespace godot;
using namespace godot::structural_inspector;

static Control* create_edit_overloaded(
		ResourceInspectorProperty* root,
		ResourceEditor* parent,
		const Schema* schema,
		const Variant& key) {
	if (auto sch = dynamic_cast<const StructSchema*>(schema)) {
		auto edit = StructEditor::_new();
		edit->_custom_init(root, parent, sch, key);
		return edit;
	} else if (auto sch = dynamic_cast<const ArraySchema*>(schema)) {
		auto edit = ArrayEditor::_new();
		edit->_custom_init(root, parent, sch, key);
		return edit;
	} else {
		auto edit = ValueEditor::_new();
		edit->_custom_init(root, parent, schema, key);
		return edit;
	}
}

void StructEditor::_register_methods() {
}

void StructEditor::_init() {
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
	this->key = key;

	if (key.get_type() == Variant::STRING) {
		title->set_text(key);
	}

	for (auto& [name, field] : schema->fields) {
		fields->add_child(create_edit_overloaded(root, this, field.get(), name));
	}
}

void StructEditor::propagate_value_update(const Variant& value) {
	root->push_key(key);
	if (parent) {
		parent->propagate_value_update(value);
	} else {
		root->set_current_value(value);
	}
}

void StructEditor::update_from(const Variant& value) {
	if (value.get_type() != Variant::DICTIONARY) {
		return;
	}

	Dictionary dict = value;
	int i = 0;
	for (auto& [name, _] : schema->fields) {
		reinterpret_cast<ResourceEditor*>(fields->get_child(i))->update_from(dict[name]);
		++i;
	}
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
	int idx = elements->get_child_count();
	auto element = create_edit_overloaded(root, this, schema->element_type.get(), idx);

	// TODO fix
	root->push_key(idx);
	schema->update_value(root, idx);

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
		// TODO fix
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

void ArrayEditor::_custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const ArraySchema* schema, const Variant& key) {
	this->root = root;
	this->parent = parent;
	this->key = key;

	if (key.get_type() == Variant::STRING) {
		title->set_text(key);
	}

	for (int i = 0; i < schema->min_elements; ++i) {
		elements->add_child(create_edit_overloaded(root, this, schema->element_type.get(), i));
	}
}

void ArrayEditor::propagate_value_update(const Variant& value) {
	root->push_key(key);
	if (parent) {
		parent->propagate_value_update(value);
	} else {
		root->set_current_value(value);
	}
}

void ArrayEditor::update_from(const Variant& value) {
	if (value.get_type() != Variant::ARRAY) {
		return;
	}

	Array array = value;
	if (array.size() < schema->min_elements || array.size() > schema->max_elements) {
		return;
	}

	for (int i = 0; i < array.size(); ++i) {
		reinterpret_cast<ResourceEditor*>(elements->get_child(i))->update_from(array[i]);
	}
}

ArrayEditor::ArrayEditor() {
}

ArrayEditor::~ArrayEditor() {
}

void ValueEditor::_register_methods() {
}

void ValueEditor::_init() {
}

void ValueEditor::_custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const Schema* schema, const Variant& key) {
	this->root = root;
	this->parent = parent;

	if (key.get_type() == Variant::STRING) {
		label = Label::_new();
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
		label->set_text(key);
		add_child(label);
	}

	if (auto sch = dynamic_cast<const StringSchema*>(schema)) {
		// TODO pattern filtering
		edit = LineEdit::_new();
	} else if (auto sch = dynamic_cast<const EnumSchema*>(schema)) {
		auto edit = OptionButton::_new();
		this->edit = edit;
		for (auto& [name, id] : sch->elements) {
			edit->get_popup()->add_item(name, id);
		}
	} else if (auto sch = dynamic_cast<const IntSchema*>(schema)) {
		auto edit = SpinBox::_new();
		this->edit = edit;
		edit->set_min(sch->min_value);
		edit->set_max(sch->max_value);
	} else if (auto sch = dynamic_cast<const FloatSchema*>(schema)) {
		auto edit = SpinBox::_new();
		this->edit = edit;
		edit->set_min(sch->min_value);
		edit->set_max(sch->max_value);
	} else if (auto sch = dynamic_cast<const BoolSchema*>(schema)) {
		edit = CheckBox::_new();
	}
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	add_child(edit);
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

	editor = create_edit_overloaded(this, nullptr, schema, Variant{});
	editor->set_visible(false);
}

Variant ResourceInspectorProperty::get_current_value() {
	auto current = get_edited_object()->get(get_edited_property());
	for (int i = keys_path.size() - 1; i >= 1; ++i) {
		auto key = keys_path[i];
		switch (key.get_type()) {
			case Variant::STRING: {
				Dictionary dict = current;
				current = dict[key];
			} break;
			case Variant::INT: {
				Array array = current;
				current = array[key];
			} break;
			default: {
				ERR_PRINT("Invalid key type " + format_variant(key.get_type()) + " when reading property.");
				return;
			}
		}
	}
	return current;
}

void ResourceInspectorProperty::set_current_value(const Variant& value) {
	auto current = get_edited_object()->get(get_edited_property());
	for (int i = keys_path.size() - 1; i >= 1; ++i) {
		auto key = keys_path[i];
		switch (key.get_type()) {
			case Variant::STRING: {
				Dictionary dict = current;
				current = dict[key];
			} break;
			case Variant::INT: {
				Array array = current;
				current = array[key];
			} break;
			default: {
				ERR_PRINT("Invalid key type " + format_variant(key.get_type()) + " when setting property.");
				return;
			}
		}
	}

	auto key = keys_path[0];
	switch (key.get_type()) {
		case Variant::STRING: {
			Dictionary dict = current;
			dict[key] = value;
		} break;
		case Variant::INT: {
			Array array = current;
			array[key] = value;
		} break;
		default: {
			ERR_PRINT("Invalid key type " + String::num_int64(key.get_type()) + " when reading property.");
			return;
		}
	}
}

void ResourceInspectorProperty::push_key(const Variant& key) {
	keys_path.append(key);
}

void ResourceInspectorProperty::clear_keys() {
	keys_path.clear();
}

void ResourceInspectorProperty::update_property() {
	updating = true;
	reinterpret_cast<ResourceEditor*>(editor)->update_from(get_edited_object()->get(get_edited_property()));
	updating = false;
}

ResourceInspectorProperty::ResourceInspectorProperty() {
}

ResourceInspectorProperty::~ResourceInspectorProperty() {
}
