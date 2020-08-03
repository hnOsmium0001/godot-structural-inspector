#include "SchemaEditor.hpp"

#include <GlobalConstants.hpp>
#include <InputEvent.hpp>
#include <InputEventMouseButton.hpp>
#include <InputEventMouseMotion.hpp>
#include <JSON.hpp>
#include <MenuButton.hpp>
#include <PopupMenu.hpp>
#include <SpinBox.hpp>
#include <algorithm>
#include <utility>

using namespace godot;
using namespace godot::structural_inspector;

DefinitionReference::DefinitionReference() :
		data{ nullptr } {}

DefinitionReference::DefinitionReference(std::vector<StructSchema::Field>* data, size_t idx) :
		data{ std::pair<std::vector<StructSchema::Field>*, size_t>{ data, idx } } {}

DefinitionReference::DefinitionReference(std::unique_ptr<Schema>* data) :
		data{ data } {}

bool DefinitionReference::has_name() {
	return data.index() == 0;
}

String DefinitionReference::get_name() {
	switch (data.index()) {
		case 0: {
			auto& data = std::get<0>(this->data);
			return (*data.first)[data.second].name;
		}
		case 1: {
			return "";
		}
	}
}

String& DefinitionReference::get_name_ref() {
	auto& data = std::get<0>(this->data);
	return (*data.first)[data.second].name;
}

std::unique_ptr<Schema>& DefinitionReference::get_schema() {
	switch (data.index()) {
		case 0: {
			auto& data = std::get<0>(this->data);
			return (*data.first)[data.second].def;
		}
		case 1: {
			auto& data = std::get<1>(this->data);
			return *data;
		}
	}
}

static const int LABEL_POS = 0;
static const int EDIT_POS = 1;

static HBoxContainer* create_nc(const String& name, Control* control) {
	auto line = HBoxContainer::_new();
	auto label = Label::_new();
	label->set_text(name);
	label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	line->add_child(label);
	control->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	line->add_child(control);
	return line;
}

static HBoxContainer* get_nc_line(Control* control) {
	return static_cast<HBoxContainer*>(control->get_parent());
}

static Label* get_nc_label(Control* control) {
	auto parent = static_cast<HBoxContainer*>(control->get_parent());
	return static_cast<Label*>(parent->get_child(LABEL_POS));
}

static VBoxContainer* create_enum_value_edit(const String& name, int id, Object* target) {
	auto enum_value = VBoxContainer::_new();
	auto binding = Array::make(enum_value);
	auto name_edit = LineEdit::_new();
	name_edit->set_text(name);
	name_edit->connect("text_changed", target, "_enum_name_set", binding);
	enum_value->add_child(::create_nc("Name", name_edit));
	auto id_edit = SpinBox::_new();
	id_edit->set_max(std::numeric_limits<int>::max());
	id_edit->set_value(id);
	id_edit->connect("value_changed", target, "_enum_id_set", binding);
	enum_value->add_child(::create_nc("ID", id_edit));
	return enum_value;
}

bool ResourceSchemaEditor::_is_mouse_inside() {
	return get_global_rect().has_point(get_global_mouse_position());
}

void ResourceSchemaEditor::_select_type(int id, Schema* swap_out) {
	if (schema_id == id) {
		return;
	}

	static const auto free_all = [](Node* node) {
		for (int i = 0; i < node->get_child_count(); ++i) {
			node->get_child(i)->queue_free();
		}
	};

	switch (schema_id) {
		case STRUCT: {
			add->set_visible(false);
			remove->set_visible(false);
			message->set_visible(false);
			free_all(list);
			list->set_visible(false);
		} break;
		case ARRAY: {
			free_all(list);
			list->set_visible(false);
			::get_nc_line(min_value)->set_visible(false);
			::get_nc_line(max_value)->set_visible(false);
		} break;
		case STRING: {
			::get_nc_line(pattern)->set_visible(false);
		} break;
		case ENUM: {
			add->set_visible(false);
			remove->set_visible(false);
			message->set_visible(false);
			free_all(list);
			list->set_visible(false);
		} break;
		case INT: {
			::get_nc_line(min_value)->set_visible(false);
			::get_nc_line(max_value)->set_visible(false);
		} break;
		case FLOAT: {
			::get_nc_line(min_value)->set_visible(false);
			::get_nc_line(max_value)->set_visible(false);
		} break;
		case BOOL: {
			// Do nothing
		} break;
		default: {
			// Do nothing, for schema_id == UNKNOWN
		} break;
	}

	switch (id) {
		case STRUCT: {
			add->set_visible(true);
			remove->set_visible(true);
			message->set_visible(true);
			list->set_visible(true);

			if (swap_out != this->definition.get_schema().get()) {
				definition.get_schema().reset(swap_out ? swap_out : new StructSchema());
				schema_id = STRUCT;
			}
		} break;
		case ARRAY: {
			list->set_visible(true);

			::get_nc_label(min_value)->set_text("Min elements");
			::get_nc_line(min_value)->set_visible(true);
			::get_nc_label(max_value)->set_text("Max elements");
			::get_nc_line(max_value)->set_visible(true);

			ArraySchema* array;
			if (swap_out != this->definition.get_schema().get()) {
				if (swap_out) {
					array = dynamic_cast<ArraySchema*>(swap_out);
				} else {
					array = new ArraySchema();
					array->element_type = std::make_unique<StructSchema>();
				}
				// `array` will be equivlant to `definition->get()` after this
				definition.get_schema().reset(array);
				schema_id = ARRAY;
			} else {
				array = dynamic_cast<ArraySchema*>(this->definition.get_schema().get());
			}

			auto child = ResourceSchemaEditor::_new();
			child->_custom_init(root, this, { &array->element_type });
			child->connect("clicked", this, "_child_clicked", Array::make(child));
			list->add_child(child);

			min_value->set_value(array->min_elements);
			max_value->set_value(array->max_elements);
		} break;
		case STRING: {
			::get_nc_line(pattern)->set_visible(true);

			if (swap_out != this->definition.get_schema().get()) {
				definition.get_schema().reset(swap_out ? swap_out : new StringSchema());
				schema_id = STRING;
			}
		} break;
		case ENUM: {
			add->set_visible(true);
			remove->set_visible(true);
			message->set_visible(true);
			list->set_visible(true);

			if (swap_out != this->definition.get_schema().get()) {
				definition.get_schema().reset(swap_out ? swap_out : new EnumSchema());
				schema_id = ENUM;
			}
		} break;
		case INT: {
			::get_nc_label(min_value)->set_text("Min value");
			::get_nc_line(min_value)->set_visible(true);
			::get_nc_label(max_value)->set_text("Max value");
			::get_nc_line(max_value)->set_visible(true);

			IntSchema* sint;
			if (swap_out != this->definition.get_schema().get()) {
				sint = swap_out ? dynamic_cast<IntSchema*>(swap_out) : new IntSchema();
				definition.get_schema().reset(sint);
				schema_id = INT;
			} else {
				sint = dynamic_cast<IntSchema*>(this->definition.get_schema().get());
			}

			min_value->set_value(sint->min_value);
			max_value->set_value(sint->max_value);
		} break;
		case FLOAT: {
			::get_nc_label(min_value)->set_text("Min value");
			::get_nc_line(min_value)->set_visible(true);
			::get_nc_label(max_value)->set_text("Max value");
			::get_nc_line(max_value)->set_visible(true);

			FloatSchema* sfloat;
			if (swap_out != this->definition.get_schema().get()) {
				auto sfloat = swap_out ? dynamic_cast<FloatSchema*>(swap_out) : new FloatSchema();
				definition.get_schema().reset(sfloat);
				schema_id = FLOAT;
			} else {
				sfloat = dynamic_cast<FloatSchema*>(this->definition.get_schema().get());
			}

			min_value->set_value(sfloat->min_value);
			max_value->set_value(sfloat->max_value);
		} break;
		case BOOL: {
			if (swap_out != this->definition.get_schema().get()) {
				definition.get_schema().reset(swap_out ? swap_out : new BoolSchema());
				schema_id = BOOL;
			}
		} break;
		default: {
			definition.get_schema() = nullptr;
			schema_id = UNKNOWN;
			ERR_PRINT("Unknown type id " + String::num_int64(id));
		} break;
	}

	schema_id = static_cast<NodeType>(id);
	root->emit_something_changed();
}

Variant ResourceSchemaEditor::_get_key() {
	if (field_name->is_visible()) {
		return field_name->get_text();
	} else if (auto parent_schema = dynamic_cast<ArraySchema*>(parent->get_schema())) {
		return get_index();
	} else {
		return Variant{};
	}
}

void ResourceSchemaEditor::_input(Ref<InputEvent> event) {
	if (auto mb = Object::cast_to<InputEventMouseButton>(event.ptr())) {
		if (mb->get_button_index() == GlobalConstants::BUTTON_LEFT && mb->is_pressed() && _is_mouse_inside()) {
			emit_signal("clicked");
		}
		return;
	}
	if (auto mm = Object::cast_to<InputEventMouseMotion>(event.ptr())) {
		if (bool current = _is_mouse_inside(); mouse_inside != current) {
			mouse_inside = current;
			update();
		}
		return;
	}
}

void ResourceSchemaEditor::_notification(int what) {
	switch (what) {
		case NOTIFICATION_READY: {
			if (definition.has_name()) {
				::get_nc_line(field_name)->set_visible(true);
				field_name->set_text(definition.get_name());
			}

			if (auto other = dynamic_cast<StructSchema*>(definition.get_schema().get())) {
				// Update UI nodes to the correct layout only, this will not update `this->schema`
				_select_type(STRUCT, other);
				type_edit->select(STRUCT);

				for (int i = 0; i < list->get_child_count(); ++i) {
					list->get_child(i)->free();
				}
				for (int i = 0; i < other->fields.size(); ++i) {
					auto child = ResourceSchemaEditor::_new();
					child->_custom_init(root, this, { &other->fields, static_cast<size_t>(i) });
					child->connect("clicked", this, "_child_clicked", Array::make(child));
					list->add_child(child);
				}
			} else if (auto other = dynamic_cast<ArraySchema*>(definition.get_schema().get())) {
				_select_type(ARRAY, other);
				type_edit->select(ARRAY);

				min_value->set_value(other->min_elements);
				max_value->set_value(other->max_elements);

				if (list->get_child_count() > 0) {
					get_child_node(0)->free();
				}

				auto child = ResourceSchemaEditor::_new();
				child->_custom_init(root, this, { &other->element_type });
				child->connect("clicked", this, "_child_clicked", Array::make(child));
				list->add_child(child);
			} else if (auto other = dynamic_cast<StringSchema*>(definition.get_schema().get())) {
				_select_type(STRING, other);
				type_edit->select(STRING);

				if (other->pattern.is_valid()) {
					pattern->set_text(other->pattern->get_pattern());
				}
			} else if (auto other = dynamic_cast<EnumSchema*>(definition.get_schema().get())) {
				_select_type(ENUM, other);
				type_edit->select(ENUM);

				for (int i = 0; i < list->get_child_count(); ++i) {
					list->get_child(i)->free();
				}
				for (auto& [name, id] : other->elements) {
					list->add_child(::create_enum_value_edit(name, id, this));
				}
			} else if (auto other = dynamic_cast<IntSchema*>(definition.get_schema().get())) {
				_select_type(INT, other);
				type_edit->select(INT);

				min_value->set_value(other->min_value);
				max_value->set_value(other->max_value);
			} else if (auto other = dynamic_cast<FloatSchema*>(definition.get_schema().get())) {
				_select_type(FLOAT, other);
				type_edit->select(FLOAT);

				min_value->set_value(other->min_value);
				max_value->set_value(other->max_value);
			} else if (auto other = dynamic_cast<BoolSchema*>(definition.get_schema().get())) {
				_select_type(BOOL, other);
				type_edit->select(BOOL);
			}
		} break;

		case NOTIFICATION_RESIZED: {
			update();
		} break;
		case NOTIFICATION_DRAW: {
			// First is regular border color, second is hovered border color
			static const Color colors[] = { Color::hex(0x262C3BFF), Color::hex(0x333B4FFF) };
			draw_rect(Rect2{ 4, 4, get_size().x - 4 * 2, get_size().y - 4 * 2 }, colors[mouse_inside], false, 2.0F);
		} break;
		default: {
		} break;
	}
}

void ResourceSchemaEditor::_type_selected(int id) {
	_select_type(id, nullptr);
}

void ResourceSchemaEditor::_add_list_item() {
	switch (schema_id) {
		case STRUCT: {
			auto schema = dynamic_cast<StructSchema*>(this->definition.get_schema().get());
			int idx = schema->fields.size();
			schema->fields.push_back({ "", std::make_unique<StructSchema>() });

			auto child = ResourceSchemaEditor::_new();
			child->_custom_init(root, this, { &schema->fields, static_cast<size_t>(idx) });
			child->connect("clicked", this, "_child_clicked", Array::make(child));
			list->add_child(child);

			root->emit_something_changed();
		} break;
		case ENUM: {
			int idx = list->get_child_count(); // Is equal to `enum_value->get_index()`
			auto schema = dynamic_cast<EnumSchema*>(this->definition.get_schema().get());
			// Default ID is its index
			schema->elements.push_back({ "", idx });

			auto enum_value = ::create_enum_value_edit("", idx, this);
			list->add_child(enum_value);

			root->emit_something_changed();
		}
		default: {
			return;
		}
	}
}

void ResourceSchemaEditor::_toggle_remove_mode() {
	switch (schema_id) {
		case STRUCT:
		case ENUM: {
			if (removing_child) {
				message->set_text("");
				removing_child = false;
			} else {
				message->set_text("Click a child property to remove.");
				removing_child = true;
			}
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaEditor::_field_name_set(const String& name) {
	if (definition.has_name()) {
		// definition.get_name_ref() = name;
		auto copy = name;
		definition.get_name_ref() = name;
		root->emit_something_changed();
	}
}

void ResourceSchemaEditor::_min_value_set(real_t value) {
	switch (schema_id) {
		case ARRAY: {
			auto schema = dynamic_cast<ArraySchema*>(this->definition.get_schema().get());
			schema->min_elements = static_cast<int>(value);
			root->emit_something_changed();
		} break;
		case INT: {
			auto schema = dynamic_cast<IntSchema*>(this->definition.get_schema().get());
			schema->min_value = static_cast<int>(value);
			root->emit_something_changed();
		} break;
		case FLOAT: {
			auto schema = dynamic_cast<FloatSchema*>(this->definition.get_schema().get());
			schema->min_value = value;
			root->emit_something_changed();
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaEditor::_max_value_set(real_t value) {
	switch (schema_id) {
		case ARRAY: {
			auto schema = dynamic_cast<ArraySchema*>(this->definition.get_schema().get());
			schema->max_elements = static_cast<int>(value);
			root->emit_something_changed();
		} break;
		case INT: {
			auto schema = dynamic_cast<IntSchema*>(this->definition.get_schema().get());
			schema->max_value = static_cast<int>(value);
			root->emit_something_changed();
		} break;
		case FLOAT: {
			auto schema = dynamic_cast<FloatSchema*>(this->definition.get_schema().get());
			schema->max_value = value;
			root->emit_something_changed();
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaEditor::_pattern_set(const String& pattern) {
	switch (schema_id) {
		case STRING: {
			auto schema = dynamic_cast<StringSchema*>(this->definition.get_schema().get());
			schema->pattern = Ref{ RegEx::_new() };
			root->emit_something_changed();
		} break;
		default: {
			return;
		}
	}
}

void ResourceSchemaEditor::_child_clicked(ResourceSchemaEditor* child) {
	if (!removing_child) {
		return;
	}

	bool removed = false;
	switch (schema_id) {
		case STRUCT: {
			auto schema = dynamic_cast<StructSchema*>(this->definition.get_schema().get());
			child->queue_free();
			schema->fields.erase(schema->fields.begin() + child->get_index());
			removed = true;
		} break;
		case ENUM: {
			auto schema = dynamic_cast<EnumSchema*>(this->definition.get_schema().get());
			child->queue_free();
			schema->elements.erase(schema->elements.begin() + child->get_index());
			removed = true;

			root->emit_something_changed();
		} break;
		default: {
			return;
		}
	}

	if (removed) {
		message->set_text("");
		removing_child = false;

		root->emit_something_changed();
	}
}

void ResourceSchemaEditor::_enum_name_set(const String& name, Control* child) {
	if (auto schema = dynamic_cast<EnumSchema*>(this->definition.get_schema().get())) {
		schema->elements[child->get_index()].name = name;
		root->emit_something_changed();
	}
}

void ResourceSchemaEditor::_enum_id_set(int id, Control* child) {
	if (auto schema = dynamic_cast<EnumSchema*>(this->definition.get_schema().get())) {
		schema->elements[child->get_index()].id = id;
		root->emit_something_changed();
	}
}

void ResourceSchemaEditor::_register_methods() {
	register_method("_input", &ResourceSchemaEditor::_input);
	register_method("_notification", &ResourceSchemaEditor::_notification);
	register_method("_type_selected", &ResourceSchemaEditor::_type_selected);
	register_method("_add_list_item", &ResourceSchemaEditor::_add_list_item);
	register_method("_toggle_remove_mode", &ResourceSchemaEditor::_toggle_remove_mode);
	register_method("_field_name_set", &ResourceSchemaEditor::_field_name_set);
	register_method("_min_value_set", &ResourceSchemaEditor::_min_value_set);
	register_method("_max_value_set", &ResourceSchemaEditor::_max_value_set);
	register_method("_pattern_set", &ResourceSchemaEditor::_pattern_set);
	register_method("_child_clicked", &ResourceSchemaEditor::_child_clicked);
	register_method("_enum_name_set", &ResourceSchemaEditor::_enum_name_set);
	register_method("_enum_id_set", &ResourceSchemaEditor::_enum_id_set);

	register_signal<ResourceSchemaEditor>("clicked", Dictionary{});
}

void ResourceSchemaEditor::_init() {
	add_constant_override("margin_left", 8);
	add_constant_override("margin_top", 8);
	add_constant_override("margin_right", 8);
	add_constant_override("margin_bottom", 8);

	contents = VBoxContainer::_new();
	add_child(contents);

	type_edit = OptionButton::_new();
	type_edit->get_popup()->add_item("Struct", STRUCT);
	type_edit->get_popup()->add_item("Array", ARRAY);
	type_edit->get_popup()->add_item("String", STRING);
	type_edit->get_popup()->add_item("Enum", ENUM);
	type_edit->get_popup()->add_item("Int", INT);
	type_edit->get_popup()->add_item("Float", FLOAT);
	type_edit->get_popup()->add_item("Bool", BOOL);
	type_edit->connect("item_selected", this, "_type_selected");
	contents->add_child(::create_nc("Type", type_edit));

	field_name = LineEdit::_new();
	field_name->connect("text_changed", this, "_field_name_set");
	auto field_name_line = ::create_nc("Field name", field_name);
	field_name_line->set_visible(false);
	contents->add_child(field_name_line);

	toolbar = HBoxContainer::_new();
	contents->add_child(toolbar);
	add = NXButton::_new();
	add->_custom_init("Add");
	add->set_visible(false);
	add->connect("pressed", this, "_add_list_item");
	toolbar->add_child(add);
	remove = NXButton::_new();
	remove->_custom_init("Remove");
	remove->set_visible(false);
	remove->connect("pressed", this, "_toggle_remove_mode");
	toolbar->add_child(remove);
	message = Label::_new();
	message->set_visible(false);
	toolbar->add_child(message);

	min_value = SpinBox::_new();
	min_value->set_min(std::numeric_limits<int>::min());
	min_value->set_max(std::numeric_limits<int>::max());
	min_value->connect("value_changed", this, "_min_value_set");
	auto min_value_line = ::create_nc("Min value", min_value);
	min_value_line->set_visible(false);
	contents->add_child(min_value_line);

	max_value = SpinBox::_new();
	max_value->set_min(std::numeric_limits<int>::min());
	max_value->set_max(std::numeric_limits<int>::max());
	max_value->connect("value_changed", this, "_max_value_set");
	auto max_value_line = ::create_nc("Max value", max_value);
	max_value_line->set_visible(false);
	contents->add_child(max_value_line);

	pattern = LineEdit::_new();
	pattern->connect("text_changed", this, "_pattern_set");
	auto pattern_line = ::create_nc("Pattern", pattern);
	pattern_line->set_visible(false);
	contents->add_child(pattern_line);

	list = VBoxContainer::_new();
	list->set_visible(false);
	auto padding_box = MarginContainer::_new();
	padding_box->add_constant_override("margin_right", 0);
	padding_box->add_constant_override("margin_top", 0);
	padding_box->add_constant_override("margin_left", 16);
	padding_box->add_constant_override("margin_bottom", 0);
	padding_box->add_child(list);
	contents->add_child(padding_box);
}

void ResourceSchemaEditor::_custom_init(ResourceSchemaInspectorProperty* root, ResourceSchemaEditor* parent, DefinitionReference definition) {
	this->root = root;
	this->parent = parent;
	this->definition = definition;

	// Updating the UI according to the incoming schema is done in _notification::READY when the child nodes are initialized
}

String ResourceSchemaEditor::get_field_name() const {
	return field_name->get_text();
}

void ResourceSchemaEditor::set_field_name(const String& name) {
	::get_nc_line(field_name)->set_visible(true);
	field_name->set_text(name);
}

ResourceSchemaEditor* ResourceSchemaEditor::get_child_node(int i) {
	return Object::cast_to<ResourceSchemaEditor>(list->get_child(i));
}

Schema* ResourceSchemaEditor::get_schema() {
	return definition.get_schema().get();
}

ResourceSchemaEditor::ResourceSchemaEditor() {
}

ResourceSchemaEditor::~ResourceSchemaEditor() {
}

void ResourceSchemaInspectorProperty::_toggle_editor_visibility() {
	if (properties->is_visible()) {
		remove_child(properties);
		set_bottom_editor(nullptr);
		properties->set_visible(false);
	} else {
		add_child(properties);
		set_bottom_editor(properties);
		properties->set_visible(true);
	}
}

void ResourceSchemaInspectorProperty::_prop_clicked(ResourceSchemaEditor* node) {
	selected_idx = node->get_index();
}

void ResourceSchemaInspectorProperty::_register_methods() {
	register_method("_toggle_editor_visibility", &ResourceSchemaInspectorProperty::_toggle_editor_visibility);
	register_method("_prop_clicked", &ResourceSchemaInspectorProperty::_prop_clicked);
	register_method("add_root_property", &ResourceSchemaInspectorProperty::add_root_property);
	register_method("remove_root_property", &ResourceSchemaInspectorProperty::remove_root_property);
	register_method("emit_something_changed", &ResourceSchemaInspectorProperty::emit_something_changed);
	register_method("update_property", &ResourceSchemaInspectorProperty::update_property);
}

void ResourceSchemaInspectorProperty::_init() {
	btn = Button::_new();
	btn->set_text("Properties");
	btn->connect("pressed", this, "_toggle_editor_visibility");
	add_child(btn);

	properties = VBoxContainer::_new();
	properties->set_visible(false);

	auto toolbar = HBoxContainer::_new();
	auto add = NXButton::_new();
	add->_custom_init("Add");
	add->connect("pressed", this, "add_root_property");
	toolbar->add_child(add);
	auto remove = NXButton::_new();
	remove->_custom_init("Remove");
	remove->connect("pressed", this, "remove_root_property");
	toolbar->add_child(remove);

	properties->add_child(toolbar);
}

ResourceSchemaEditor* ResourceSchemaInspectorProperty::add_root_property_with(const String& name, std::unique_ptr<Schema> schema) {
	schemas.push_back({ name, std::move(schema) });

	auto prop = ResourceSchemaEditor::_new();
	prop->_custom_init(this, nullptr, { &schemas, schemas.size() - 1 });
	prop->connect("clicked", this, "_prop_clicked", Array::make(prop));
	properties->add_child(prop);
	emit_something_changed();
	return prop;
}

ResourceSchemaEditor* ResourceSchemaInspectorProperty::add_root_property() {
	return add_root_property_with("", std::make_unique<StructSchema>());
}

void ResourceSchemaInspectorProperty::remove_root_property() {
	if (selected_idx != -1) {
		properties->get_child(selected_idx)->free();
		selected_idx = -1;
	}
	emit_something_changed();
}

void ResourceSchemaInspectorProperty::emit_something_changed() {
	if (updating) return;

	Array saved_props;
	for (int i = 1; i < properties->get_child_count(); ++i) {
		if (auto prop = Object::cast_to<ResourceSchemaEditor>(properties->get_child(i))) {
			auto data = save_schema(prop->get_schema());
			data["name"] = prop->get_field_name();
			saved_props.append(data);
		} else {
			ERR_PRINT("Properties box contains unexpected nodes that aren't of type ResourceSchemaEditor");
		}
	}

	emit_changed(get_edited_property(), saved_props, "", true);
}

void ResourceSchemaInspectorProperty::update_property() {
	updating = true;

	for (int i = 1; i < properties->get_child_count(); ++i) {
		properties->get_child(i)->free();
	}

	Array data = get_edited_object()->get(get_edited_property());
	for (int i = 0; i < data.size(); ++i) {
		Dictionary dict = data[i];
		// If parsing schema failed, we skip this entry
		if (auto schema = parse_schema(dict)) {
			add_root_property_with(dict["name"], std::move(schema));
		} else {
			ERR_PRINT("Error while parsing schema entry: " + JSON::get_singleton()->print(dict));
		}
	}

	updating = false;
}

ResourceSchemaInspectorProperty::ResourceSchemaInspectorProperty() {
}

ResourceSchemaInspectorProperty::~ResourceSchemaInspectorProperty() {
}
