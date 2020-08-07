#pragma once

#include <Button.hpp>
#include <Godot.hpp>
#include <MarginContainer.hpp>
#include <String.hpp>
#include <functional>
#include <iterator>
#include <memory>

namespace godot::structural_inspector {

template <class... Ts>
struct Overloaded : Ts... { using Ts::operator()...; };
template <class... Ts>
Overloaded(Ts...)->Overloaded<Ts...>;

template <class This>
class CloneProvider {
public:
	virtual This* clone() const = 0;

	std::unique_ptr<This> clone_uptr() const {
		return std::unique_ptr<This>(clone());
	}
};

template <class T>
class Iterator {
public:
	virtual bool has_next() const = 0;
	virtual T next() = 0;
};

class EditorIconButton : public Button {
	GODOT_CLASS(EditorIconButton, Button)
private:
	String icon_name;

	void _notification(int what);
	void _apply_icon();

public:
	static void _register_methods();
	void _init();
	void _custom_init(const String& icon_name);

	EditorIconButton();
	~EditorIconButton();
};

class BorderedContainer : public MarginContainer {
	GODOT_CLASS(BorderedContainer, MarginContainer)
private:
	bool mouse_inside;

	void _notification(int what);
	void _input(Ref<InputEvent> event);

public:
	static void _register_methods();
	void _init();

	BorderedContainer();
	~BorderedContainer();
};

// class ListContainer : public Container {
// 	GODOT_CLASS(ListContainer, Container)
// public:
// 	enum Direction {
// 		HORIZONTAL,
// 		VERTICAL,
// 	};

// private:
// 	Direction dir = VERTICAL;
// 	int separation = 4;
// 	bool stretch = false;

// 	void _reorder();

// 	void _notification(int what);

// public:
// 	static void _register_methods();
// 	void _init();

// 	Direction get_direction() const;
// 	void set_direction(Direction dir);

// 	int get_separation() const;
// 	void set_separation(int separation);

// 	bool is_stretching() const;
// 	void set_stretching(bool stretching);

// 	// Size2 get_minimum_size();

// 	ListContainer();
// 	~ListContainer();
// };

String format_variant(const Variant& variant);

} // namespace godot::structural_inspector

template <>
struct std::hash<godot::String> {
	size_t operator()(const godot::String& str) const noexcept;
};
