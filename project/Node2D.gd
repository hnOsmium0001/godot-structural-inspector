extends Node2D


# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
	if ListContainer == null:
		print("nulls")
	var a = ListContainer.new()
	a.add_child(make_label())
	a.add_child(make_label())
	a.add_child(make_label())
	add_child(a)
	pass # Replace with function body.

func make_label():
	var l := Label.new()
	l.text = "test"
	return l

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
