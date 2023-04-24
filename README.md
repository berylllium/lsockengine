# LiSE
A PS1-style game engine.

## Table of Contents

- [About](#about)
- [Support](#support)
- [Notice](#notice)

## About

LiSE (short for Lipin Sock Engine) is a game engine with built-in support for PS-1 graphics, but can be extended to support modern rendering techniques as well.

LiSE is a very transparent engine. The consumer of the engine is expected to know how the engine works from the inside, and the engine does very little to abstract away any tasks or concepts. Because of the transparent nature of the engine, the documentation and even this very readme is filled with direct copy-pastes of source code to explain certain concepts.

Below you'll find more information about relevant systems and concepts.

### Node system

LiSE uses a node system which is heavily inspired by the [Godot Engine](https://godotengine.org/)s nodes. This readme will provide a short explanation of nodes. For a more in-depth explanation and examples, please view the official documentation.

#### Nodes

A node is simply an object containing references to other nodes. To be specific; a node always contains a reference to __one__ parent (an exception to this rule is root nodes, the root node never has a parent, only when imported in other nodes, importing nodes will be explained later), and a varying amount of references to children. Besides references to other objects, a node object also contains the name of the node. This way, it is possible to create a hierarchy of nodes. See the following example of a node:

```
o node_0		(node)
│
├ o node_1		(node)
│ └ o node_2	(node)
│
└ o node_3	 	(node)
```

Here we see three nodes. `node_0` has no parent, because it is the root node. `node_0` has two children: `node_1` and `node_3`. `node_0` is also the parent of both `node_1` and `node_3`. `node_2` is the child of `node_1`, also making `node_1` the parent of `node_2`.

#### Types of nodes

You might've noticed the word "node" in parenthesis in the example above. This is the type of the node. The engine comes with a bunch of built-in node types. Node types extend eachother. Every type of node is always derived from the "node" node type; the "node" type is the basis of every other type.

We'll use the "spatial" node as an example. A spatial node is the basis of all nodes that represent objects in three-dimentional space. Let's first take a look at the implementation of the "node" type node.

```c
typedef struct lise_node
{
	char* name;

	lise_node_abstract parent_note;

	blib_darray children; // lise_node_abstract
} lise_node;
```

Each "node" type node stores a name, a reference to a parent, and a dynamic array of children.

Now let's take a look at the implementation of the "mesh_renderer" type node.

```c
typedef struct lise_node_spatial
{
	lise_node node;

	lise_transform transform;
} lise_node_spatial;
```

We can see that the "spatial" type node is an extention of the "node" type node. We can also see that the "spatial" type node contains a "transform". A transform representing an objects position and orientation in three-dimentional space.

## Support

LiSE provides native support for Windows and GNU/Linux systems using the Xorg window system. Support for the Wayland window system is planned, while support for MacOs is not.

### Windows

- Download and install the [Vulkan SDK](https://vulkan.lunarg.com/). (Preferably the same version used in the project, currently version 1.3.231.1. You could probably use more recent versions.)
- Change the `VULKAN_SDK` variable in the CMakeLists file to where you've installed the SDK. Also change the `VULKAN_VERSION` variable if you've chosen to go for a more recent version.
- Configure the CMake project and build it.

### GNU/Linux

The following instructions apply to Arch Linux. Other distributions have different names for the packages.

- Install the following packages from pacman: `vulkan-devel shaderc`. (`vulkan-devel` contains everything needed for vulkan development, including Vulkan headers and the library required for linking. The `shaderc` package contains the `glslc` binary used to compile GLSL code into SPIR-V bytecode.)
- Configure the CMake project and build it.

> **__NOTE:__** Please read through the [Notice](#notice) section for more information about the quirks of running LiSE on GNU/Linux using the Xorg window system.

## Notice

LiSE uses `= {}` struct initialization. At the time of writing, this is a GNU C extension. C23 will include this as standard in the future.

LiSE uses the `__VA_OPT__` function macro. At the time of writing, this is a GNU C extension. C23 will include this as standard in the future.

Xorg does not support disabling key-repeats for specific windows. This means that when one wants to disable key-repeats they have to do so for the entire window system. Meaning LiSE will disable key-repeats for the entire system and reenable it when closing down. This also means that key-repeats will remain disabled if the program closes without successfully going through the shutdown procedure; this may occur on crashes.
