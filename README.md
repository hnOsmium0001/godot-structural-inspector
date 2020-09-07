# Godot Structural Insector

Godot Structural Inspector is a Godot 3+ plugin that allows editing resources using a "structural editor" derived from schema files.

## Features

+ Creating schema files for scipts that inherit from `Resource` (`ResourceScript.gd` -> `ResourceScript.schema.tres`)
+ Editing schema files in a custom, friendly inspector
+ Edit schema-ed files with validation

## Detailed example

For example, say there is a `CharacterTraits.gd` resource, and it has the following properties (using non-gdscript type notation just to make things clear)
```
name: String
health: int
stamina: int
traits: Array<String>
realted_characters: Array<struct { name: String; relation: String; }>
```
If you were using the built-in resource editor, editing this could get cumbersome really quickly. Peeking into the dictionaries ("struct") inside arrays requires many clicks, adding an element requires going through a giant list of types, etc..
By defining a schema file named `CharacterTraits.schema.tres`, the complex fields will be more readable, and all modifications will be validated.

## Screenshots

WIP
