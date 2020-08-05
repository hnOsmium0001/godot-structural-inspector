#include "ResourceEditor.hpp"

#include <CheckBox.hpp>
#include <InputEvent.hpp>
#include <InputEventMouseButton.hpp>
#include <PopupMenu.hpp>
#include <utility>

using namespace godot;
using namespace godot::structural_inspector;

void ResourceEditor::write(const Variant& value) {
	root->push_key(key);
	if (parent) {
		parent->write(value);
	} else {
		root->set_current_value(value);
	}
}

void ResourceEditor::write(const std::function<auto(const Variant&)->Variant>& mapper) {
	root->push_key(key);
	if (parent) {
		parent->write(mapper);
	} else {
		auto& value = root->get_current_value();
		root->set_current_value(mapper(value));
	}
}

Variant ResourceEditor::get_key() const {
	return key;
}

void ResourceEditor::set_key(const Variant& value) {
	this->key = key;
}

static std::pair<Control*, ResourceEditor*> create_edit_as_iface(
		ResourceInspectorProperty* root,
		ResourceEditor* parent,
		const Schema* schema,
		const Variant& key) {
	if (auto sch = dynamic_cast<const StructSchema*>(schema)) {
		auto edit = StructEditor::_new();
		edit->_custom_init(root, parent, sch, key);
		return { edit, edit };
	} else if (auto sch = dynamic_cast<const ArraySchema*>(schema)) {
		auto edit = ArrayEditor::_new();
		edit->_custom_init(root, parent, sch, key);
		return { edit, edit };
	} else {
		auto edit = ValueEditor::_new();
		edit->_custom_init(root, parent, schema, key);
		return { edit, edit };
	}
}

static Control* create_edit_overloaded(
		ResourceInspectorProperty* root,
		ResourceEditor* parent,
		const Schema* schema,
		const Variant& key) {
	return create_edit_as_iface(root, parent, schema, key).first;
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

void StructEditor::read(const Variant& value) {
	if (value.get_type() != Variant::DICTIONARY) {
		return;
	}

	Dictionary dict = value;
	int i = 0;
	for (auto& [name, _] : schema->fields) {
		reinterpret_cast<ResourceEditor*>(fields->get_child(i))->read(dict[name]);
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
	element->connect("gui_input", this, "_element_gui_input", Array::make(element));

	write([&](const Variant& value) {
		Array array = value;
		array.insert(idx, Variant{});
		return array;
	});

	elements->add_child(element);
	if (selected_idx != -1 && selected_idx < elements->get_child_count() - 1) {
		elements->move_child(element, selected_idx + 1);
		for (int i = selected_idx + 2; i < elements->get_child_count(); ++i) {
			reinterpret_cast<ResourceEditor*>(elements->get_child(i))->set_key(i);
		}
	}

	if (elements->get_child_count() == schema->max_elements) {
		add->set_disabled(true);
	}
}

void ArrayEditor::_remove_element() {
	if (selected_idx != -1) {
		write([&](const Variant& value) {
			Array array = value;
			array.remove(selected_idx);
			return array;
		});

		elements->get_child(selected_idx)->free();
		// The elements after the freed element was moved forward, as we used free() instead of queue_free()
		for (int i = selected_idx; i < elements->get_child_count(); ++i) {
			reinterpret_cast<ResourceEditor*>(elements->get_child(i))->set_key(i - 1);
		}

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

void ArrayEditor::read(const Variant& value) {
	if (value.get_type() != Variant::ARRAY) {
		return;
	}

	Array array = value;
	if (array.size() < schema->min_elements || array.size() > schema->max_elements) {
		return;
	}

	for (int i = 0; i < array.size(); ++i) {
		reinterpret_cast<ResourceEditor*>(elements->get_child(i))->read(array[i]);
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
	this->schema = schema;
	this->key = key;

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

void ValueEditor::read(const Variant& value) {
	if (auto sch = dynamic_cast<const StringSchema*>(schema) && value.get_type() == Variant::STRING) {
		static_cast<LineEdit*>(edit)->set_text(value);
	} else if (auto sch = dynamic_cast<const EnumSchema*>(schema) && value.get_type() == Variant::INT) {
		static_cast<OptionButton*>(edit)->select(value);
	} else if (auto sch = dynamic_cast<const IntSchema*>(schema) && value.get_type() == Variant::INT) {
		static_cast<SpinBox*>(edit)->set_value(value);
	} else if (auto sch = dynamic_cast<const FloatSchema*>(schema) && value.get_type() == Variant::REAL) {
		static_cast<SpinBox*>(edit)->set_value(value);
	} else if (auto sch = dynamic_cast<const BoolSchema*>(schema) && value.get_type() == Variant::BOOL) {
		static_cast<OptionButton*>(edit)->select(value ? 1 : 0);
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

	auto pair = create_edit_as_iface(this, nullptr, schema, Variant{});
	ieditor = pair.second;
	editor = pair.first;
	editor->set_visible(false);
}

Variant ResourceInspectorProperty::get_current_value() {
	switch (staging_key.get_type()) {
		case Variant::STRING: {
			Dictionary dict = staging_value;
			return dict[staging_key];
		}
		case Variant::INT: {
			Array array = staging_value;
			return array[staging_key];
		}
		case Variant::NIL: {
			return staging_value;
		}
		default: {
			// The incoming key was validated in push_key()
			ERR_PRINT("This should never happen. Report this bug immediately.");
			return Variant{};
		}
	}
}

void ResourceInspectorProperty::set_current_value(const Variant& value) {
	switch (staging_key.get_type()) {
		case Variant::STRING: {
			Dictionary dict = staging_value;
			dict[staging_key] = value;
		} break;
		case Variant::INT: {
			Array array = staging_value;
			array[staging_key] = value;
		} break;
		case Variant::NIL: {
			ERR_PRINT("Cannnot set the edited property directly.");
		} break;
		default: {
			// The incoming key was validated in push_key()
			ERR_PRINT("This should never happen. Report this bug immediately.");
		} break;
	}
}

void ResourceInspectorProperty::push_key(const Variant& key) {
	switch (staging_key.get_type()) {
		case Variant::STRING: {
			Dictionary dict = staging_value;
			staging_value = dict[key];
		} break;
		case Variant::INT: {
			Array array = staging_value;
			staging_value = array[key];
		} break;
		default: break;
	}

	this->staging_key = key;
	// Validate the incoming key so that we get the possible error immediately, instead of in the next push_key() invocation
	switch (key.get_type()) {
		case Variant::STRING:
		case Variant::INT: {
			// Valid key type
		} break;
		default: {
			ERR_PRINT("Invalid key type " + format_variant(key.get_type()) + ".");
		} break;
	}
}

void ResourceInspectorProperty::clear_keys() {
	staging_key = Variant{};
	staging_value = get_edited_object()->get(get_edited_property());
}

void ResourceInspectorProperty::update_property() {
	updating = true;
	// reinterpret_cast<ResourceEditor*>(editor)->read(get_edited_object()->get(get_edited_property()));
	ieditor->read(get_edited_object()->get(get_edited_property()));
	updating = false;
}

ResourceInspectorProperty::ResourceInspectorProperty() {
}

ResourceInspectorProperty::~ResourceInspectorProperty() {
}
