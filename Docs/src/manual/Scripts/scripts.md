# Scripts {#Scripts}

OGRE-Next drives many of its features through scripts in order to make it easier to set up. The scripts are simply plain text files which can be edited in any standard text editor, and modifying them immediately takes effect on your OGRE-Next-based applications, without any need to recompile. This makes prototyping a lot faster. Here are the items that OGRE-Next lets you script:

- @subpage JSON-Materials
- @subpage Material-Scripts
- @subpage High-level-Programs
- @subpage compositor [Scripts](@ref compositor)
- @subpage Particle-Scripts
- @subpage v1_Overlays

@tableofcontents

# Loading scripts

Scripts are loaded when resource groups are initialised: OGRE-Next looks in all resource locations associated with the group (see Ogre::ResourceGroupManager::addResourceLocation) for files with the respective extension (e.g. ’.material’, ’.material.json’, ’.compositor’, ..) and parses them. If you want to parse files manually, use Ogre::ScriptCompilerManager::parseScript.

The file extension does not actually restrict the items that can be specified inside the file; e.g. %Ogre is perfectly fine with loading a particle-system from a ’.compositor’ file - but it will lead you straight to maintenance-hell if you do that.
The extensions, however, do specify the order in which the scripts are parsed, which is as follows:

1. *.material.json
2. *.program
3. *.material
4. *.particle
5. *.particle2
6. *.compositor
7. *.os
8. *.overlay

# Format {#Format}

Several script objects may be defined in a single file. The script format is pseudo-C++, with sections delimited by curly braces ({}), and comments indicated by starting a line with ’//’. The general format is shown below:

```cpp
// This is a comment
object_keyword Example/ObjectName
{
    attribute_name "some value"

    object_keyword2 "Nested Object"
    {
        other_attribute 1 2 3
        // and so on..
    }
}
```

Every script object must be given a name, which is the line ’object_keyword &lt;name&gt;’ before the first opening ’{’. This name must be globally unique. It can include path characters (as in the example) to logically divide up the objects, and also to avoid duplicate names, but the engine does not treat the name as hierarchical, just as a string. Names can include spaces but must be surrounded by double quotes i.e. `compositor "My Name"`.

@note ’:’ is the delimiter for specifying inheritance in the script so it can’t be used as part of the name.

An script object can inherit from a previously defined object by using a *colon* ’:’ after the name followed by the name of the reference object to inherit from. You can in fact even inherit just *parts* of a script object from others; all this is covered in @ref Script-Inheritance). You can also use variables in your script which can be replaced in inheriting versions, see @ref Script-Variables.

## Script Inheritance {#Script-Inheritance}

When creating new script objects that are only slight variations of another object, it’s good to avoid copying and pasting between scripts. Script inheritance lets you do this; in this section we’ll use material scripts as an example, but this applies to all scripts parsed with the script compilers in %Ogre 1.6 onwards.

For example, to make a new material that is based on one previously defined, add a *colon* ’:’ after the new material name followed by the name of the material that is to be copied.

Example
```cpp
material <NewUniqueChildName> : <ReferenceParentMaterial>
```

The only caveat is that a parent material must have been defined/parsed prior to the child material script being parsed. The easiest way to achieve this is to either place parents at the beginning of the material script file, or to use the @ref Script-Import-Directive. Note that inheritance is actually a copy - after scripts are loaded into Ogre, objects no longer maintain their copy inheritance structure. If a parent material is modified through code at runtime, the changes have no effect on child materials that were copied from it in the script.

Material copying within the script alleviates some drudgery from copy/paste but having the ability to identify specific techniques, passes, and texture units to modify makes material copying easier. Techniques, passes, texture units can be identified directly in the child material without having to layout previous techniques, passes, texture units by associating a name with them, Techniques and passes can take a name and texture units can be numbered within the material script. You can also use variables, See @ref Script-Variables.

Names become very useful in materials that copy from other materials. In order to override values they must be in the correct technique, pass, texture unit etc. The script could be lain out using the sequence of techniques, passes, texture units in the child material but if only one parameter needs to change in say the 5th pass then the first four passes prior to the fifth would have to be placed in the script:

Here is an example:

```cpp
material test2 : test1
{
  technique
  {
    pass
    {
    }

    pass
    {
    }

    pass
    {
    }

    pass
    {
    }

    pass
    {
      ambient 0.5 0.7 0.3 1.0
    }
  }
}
```

This method is tedious for materials that only have slight variations to their parent. An easier way is to name the pass directly without listing the previous passes:<br>

```cpp
material test2 : test1
{
  technique
  {
    pass "Fifth Pass"
    {
      ambient 0.5 0.7 0.3 1.0
    }
  }
}
```

The parent pass name must be known and the pass must be in the correct technique in order for this to work correctly. Specifying the technique name and the pass name is the best method.

### Advanced Script Inheritance {#Advanced-Script-Inheritance}

Script objects can also inherit from each other more generally. The previous concept of inheritance, material copying, was restricted only to the top-level material objects. Now, any level of object can take advantage of inheritance (for instance, techniques, passes, and compositor targets).

```cpp
material Test
{
    technique
    {
        pass : ParentPass
        {
        }
    }
}
```

Notice that the pass inherits from ParentPass. This allows for the creation of more fine-grained inheritance hierarchies.

Along with the more generalized inheritance system comes an important new keyword: "abstract." This keyword is used at a top-level object declaration (not inside any other object) to denote that it is not something that the compiler should actually attempt to compile, but rather that it is only for the purpose of inheritance. For example, a material declared with the abstract keyword will never be turned into an actual usable material in the material framework. Objects which cannot be at a top-level in the document (like a pass) but that you would like to declare as such for inheriting purpose must be declared with the abstract keyword.

```cpp
abstract pass ParentPass
{
    diffuse 1 0 0 1
}
```

That declares the ParentPass object which was inherited from in the above example. Notice the abstract keyword which informs the compiler that it should not attempt to actually turn this object into any sort of Ogre resource. If it did attempt to do so, then it would obviously fail, since a pass all on its own like that is not valid.

The final matching option is based on wildcards. Using the ’\*’ character, you can make a powerful matching scheme and override multiple objects at once, even if you don’t know exact names or positions of those objects in the inherited object.

```cpp
abstract technique Overrider
{
   pass *colour*
   {
      diffuse 0 0 0 0
   }
}
```

This technique, when included in a material, will override all passes matching the wildcard "\*color\*" (color has to appear in the name somewhere) and turn their diffuse properties black. It does not matter their position or exact name in the inherited technique, this will match them.

## Script Variables {#Script-Variables}

A very powerful new feature in Ogre 1.6 is variables. Variables allow you to parameterize data in materials so that they can become more generalized. This enables greater reuse of scripts by targeting specific customization points. Using variables along with inheritance allows for huge amounts of overrides and easy object reuse.

```cpp
abstract pass ParentPass
{
   diffuse $diffuse_colour
}

material Test
{
   technique
   {
       pass : ParentPass
       {
           set $diffuse_colour "1 0 0 1"
       }
   }
}
```

The ParentPass object declares a variable called "diffuse\_colour" which is then overridden in the Test material’s pass. The "set" keyword is used to set the value of that variable. The variable assignment follows lexical scoping rules, which means that the value of "1 0 0 1" is only valid inside that pass definition. Variable assignment in outer scopes carry over into inner scopes.

```cpp
material Test
{
    set $diffuse_colour "1 0 0 1"
    technique
    {
        pass : ParentPass
        {
        }
    }
}
```

The $diffuse\_colour assignment carries down through the technique and into the pass. 

## Script Import Directive {#Script-Import-Directive}

Imports are a feature introduced to remove ambiguity from script dependencies. When using scripts that inherit from each other but which are defined in separate files sometimes errors occur because the scripts are loaded in incorrect order. Using imports removes this issue. The script which is inheriting another can explicitly import its parent’s definition which will ensure that no errors occur because the parent’s definition was not found.

```cpp
import * from "parent.material"
material Child : Parent
{
}
```

The material "Parent" is defined in parent.material and the import ensures that those definitions are found properly. You can also import specific targets from within a file.

```cpp
import Parent from "parent.material"
```

If there were other definitions in the parent.material file, they would not be imported.

@note Importing does not actually cause objects in the imported script to be fully parsed & created, it just makes the definitions available for inheritance. This has a specific ramification for vertex / fragment program definitions, which must be loaded before any parameters can be specified. You should continue to put common program definitions in .program files to ensure they are fully parsed before being referenced in multiple .material files. The ’import’ command just makes sure you can resolve dependencies between equivalent script definitions (e.g. material to material).

# Custom Translators {#custom-translators}
Writing a custom translators allows you to extend Ogre's standard compilers with completely new functionality. The same scripting interfaces can be used to define application-specific functionality. Here's how you do it.

The first step is creating a custom translator class which extends Ogre::ScriptTranslator.

@snippet Components/Overlay/src/OgreOverlayTranslator.h font_translator

This class defines the important function to override: translate. This is called when the TestTranslator needs to process a sub-set of the parsed script. The definition of this function might look something like this:

@snippet Components/Overlay/src/OgreOverlayTranslator.cpp font_translate

The translate function here expects all children to be atomic properties. Sub-objects can also be processed by checking if the child node type is Ogre::ANT_OBJECT.

From here you need to register the translator to be invoked when the proper object is found in the scripts. To do this we need to create a Ogre::ScriptTranslatorManager object to create your custom translator. The relevant parts look like this:

@snippet Components/Overlay/src/OgreOverlayTranslator.cpp font_register

Note how we use Ogre::ScriptCompilerManager::registerCustomWordId to avoid string comparisons in our code.

@snippet Components/Overlay/src/OgreOverlayTranslator.cpp font_get_translator

No new translators are created here, just returned when requested. This is because our translator does not require separate instances to properly parse scripts, and so it is easier to only create one instance and reuse it. Often this strategy will work.

The order that custom translator managers are registered will make a difference. When the system is attempting to find translators to handle pieces of a script, it will query the translator managers one-by-one until it finds one that handles that script object. It is a first-come-first-served basis.

An important note is that this will recognize the above pattern no matter where in the script it is. That means that this may appear at the top-level of a script or inside several sub-objects. If this is not what you want then you can change the translator manager to do more advanced processing in the getTranslator function.

@page Compositor-Scripts Compositor Scripts

The compositor framework is a subsection of the OGRE API that allows you to easily define how to assemble the final image. A typical use-case are full screen post-processing effects - however Compositors are not limited to that. Compositor scripts offer you the ability to define rendering pipelines in a script which can be reused and modified easily, rather than having to use the API to define them. You still need to use code to instantiate a compositor against one of your visible viewports, but this is a much simpler process than actually defining the compositor itself.

@tableofcontents

# Compositor Fundamentals {#Compositor-Fundamentals}

Compositing generally involves rendering the scene to a texture, either in addition to or instead of the main window. Once the scene is in a texture, you can then pull the scene image into a fragment program and perform operations on it by rendering it through full screen quad. The target of this post processing render can be the main result (e.g. a window), or it can be another render texture so that you can perform multi-stage convolutions on the image. You can even ’ping-pong’ the render back and forth between a couple of render textures to perform convolutions which require many iterations, without using a separate texture for each stage. Eventually you’ll want to render the result to the final output, which you do with a full screen quad. This might replace the whole window (thus the main window doesn’t need to render the scene itself), or it might be a combinational effect. 

So that we can discuss how to implement these techniques efficiently, a number of definitions are required:

<dl compact="compact">
<dt>Compositor</dt> <dd>

Definition of a rendering pipeline that can be applied to a user viewport. This is what you’re defining when writing compositor scripts as detailed in this section.

</dd> <dt>Compositor Instance</dt> <dd>

An instance of a compositor as applied to a single viewport. You create these based on compositor definitions, See @ref Applying-a-Compositor.

</dd> <dt>Compositor Chain</dt> <dd>

It is possible to enable more than one compositor instance on a viewport at the same time, with one compositor taking the results of the previous one as input. This is known as a compositor chain. Every viewport which has at least one compositor attached to it has a compositor chain. See @ref Applying-a-Compositor

</dd> <dt>Target</dt> <dd>

This is a Ogre::RenderTarget, i.e. the canvas where the result of a series of render operations is sent. A target may be the final output (e.g. your Ogre::RenderWindow), or it may be an intermediate render texture, which you declare in your script with the @ref compositor-texture line. A target which is not the output target has a size and pixel format which you can define inside the compositor script.

</dd> <dt>Output Target</dt> <dd>

As Target, but this is the single final result of all operations. The size and pixel format of this target cannot be controlled by the compositor since it is defined by the application using it, thus you don’t declare it in your script. However, you do declare a Target Section for it, see below.

</dd> <dt>Target Section</dt> <dd>

A Target may be rendered to many times in the course of a composition effect. In particular if you ’ping pong’ a convolution between a couple of textures, you will have more than one Target Sections per Target. Target Sections are declared in the script using a [target or target\_output line](#Compositor-Target-Passes), the latter being the final output of which there can be only one.
@note Internally this is referred to as Ogre::CompositionTargetPass

</dd> <dt>Pass</dt> <dd>

Within a Target Section, there are one or more individual @ref Compositor-Passes, which perform a very specific action, such as rendering the original scene (or pulling the result from the previous compositor in the chain), rendering a fullscreen quad, or clearing one or more buffers. Typically within a single target section you will use the either a `render_scene` pass or a `render_quad` pass, not both. Clear can be used with either type.

</dd> </dl>

@snippet Samples/Media/materials/scripts/Examples.compositor manual_sample

The major components of a compositor are the @ref Compositor-Techniques, the @ref Compositor-Target-Passes and the @ref Compositor-Passes, which are covered in detail in the following sections.

# Techniques {#Compositor-Techniques}

A compositor technique is much like a [material technique](@ref Techniques) in that it describes one approach to achieving the effect you’re looking for. A compositor definition can have more than one technique if you wish to provide some fallback should the hardware not support the technique you’d prefer to use. Techniques are evaluated for hardware support based on 2 things:

<dl compact="compact">
<dt>Material support</dt> <dd>

All @ref Compositor-Passes that render a fullscreen quad use a material; for the technique to be supported, all of the materials referenced must have at least one supported material technique. If they don’t, the compositor technique is marked as unsupported and won’t be used.

</dd> <dt>Texture format support</dt> <dd>

This one is slightly more complicated. When you request a @ref compositor-texture in your technique, you request a pixel format. Not all formats are natively supported by hardware, especially the floating point formats. However, in this case the hardware will typically downgrade the texture format requested to one that the hardware does support - with compositor effects though, you might want to use a different approach if this is the case. So, when evaluating techniques, the compositor will first look for native support for the exact pixel format you’ve asked for, and will skip onto the next technique if it is not supported, thus allowing you to define other techniques with simpler pixel formats which use a different approach. If it doesn’t find any techniques which are natively supported, it tries again, this time allowing the hardware to downgrade the texture format and thus should find at least some support for what you’ve asked for.

</dd> </dl>

As with material techniques, compositor techniques are evaluated in the order you define them in the script, so techniques declared first are preferred over those declared later.

@par
Format: technique { }

Techniques can have the following nested elements:

-   [texture](#compositor-texture)
-   [texture\_ref](#compositor_005ftexture_005fref)
-   [scheme](#compositor_005fscheme)
-   [compositor\_logic](#compositor_005flogic)
-   [target](#Compositor-Target-Passes)
-   [target\_output](#Compositor-Target-Passes)

<a name="compositor_005ftexture"></a><a name="texture-2"></a>

## texture {#compositor-texture}

This declares a render texture for use in subsequent @ref Compositor-Target-Passes.
@par
Format: texture &lt;name&gt; &lt;width&gt; &lt;height&gt; &lt;PixelFormat&gt; \[&lt;MRT Pixel_Format2&gt;\] \[&lt;MRT Pixel_FormatN&gt;\] \[pooled\] \[gamma\] \[no\_fsaa\] \[depth\_pool &lt;poolId&gt;\] \[&lt;scope&gt;\] \[&lt;cubic&gt;\]

@param name
A name to give the render texture, which must be unique within this compositor. This name is used to reference the texture in @ref Compositor-Target-Passes, when the texture is rendered to, and in @ref Compositor-Passes, when the texture is used as input to a material rendering a fullscreen quad.

@param width
@param height
@parblock
The dimensions of the render texture. You can either specify a fixed width and height, or you can request that the texture is based on the physical dimensions of the viewport to which the compositor is attached. The options for the latter are either of
<ul>
<li> @c target_width and @c target_height
<li> @c target_width_scaled &lt;factor&gt; and @c target_height_scaled &lt;factor&gt;
</ul>
where &lt;factor&gt; is the amount by which you wish to multiply the size of the main target to derive the dimensions.
@endparblock
@param PixelFormat
The Ogre::PixelFormat of the render texture. This affects how much memory it will take, what colour channels will be available, and what precision you will have within those channels.
You can in fact repeat this element if you wish. If you do so, that means that this render texture becomes a Multiple Render Target (MRT), when the GPU writes to multiple textures at once.
If you use a depth format here, the texture will be used as the depth attachment instead of the depth-buffer. Use the MRT format to specify both the depth and the colour attachments.


@param pooled
If present, this directive makes this texture ’pooled’ among compositor instances, which can save some memory.

@param gamma
If present, this directive means that sRGB gamma correction will be enabled on writes to this texture. You should remember to include the opposite sRGB conversion when you read this texture back in another material, such as a quad. This option will automatically enabled if you use a render\_scene pass on this texture and the viewport on which the compositor is based has sRGB write support enabled.

@param no\_fsaa
If present, this directive disables the use of anti-aliasing on this texture. FSAA is only used if this texture is subject to a render\_scene pass and FSAA was enabled on the original viewport on which this compositor is based; this option allows you to override it and disable the FSAA if you wish.

@param depth\_pool
When present, this directive has to be followed by an integer. This directive is unrelated to the "pooled" directive. This one sets from which Depth buffer pool the depth buffer will be chosen from. All RTs from all compositors (including render windows if the render system API allows it) with the same pool ID share the same depth buffers (following the rules of the current render system APIs, (check RenderSystemCapabilities flags to find the rules). When the pool ID is 0, no depth buffer is used. This can be helpful for passes that don’t require a Depth buffer at all, potentially saving performance and memory. Default value is 1.
Ignored with depth pixel formats.

@param scope
If present, this directive sets the scope for the texture for being accessed by other compositors using the [texture\_ref](#compositor_005ftexture_005fref) directive. There are three options : 
1. @c local_scope (which is also the default) means that only the compositor defining the texture can access it. 
2. @c chain_scope means that the compositors after this compositor in the chain can reference its textures, and 
3. @c global_scope means that the entire application can access the texture. This directive also affects the creation of the textures (global textures are created once and thus can’t be used with the pooled directive, and can’t rely on viewport size).

@param cubic
This texture is of type Ogre::TEX_TYPE_CUBE_MAP - i.e. made up of 6 2D textures which are pasted around the inside of a cube.

@par
Example: texture rt0 512 512 PF\_R8G8B8A8
@par
Example: texture rt1 target\_width target\_height PF\_FLOAT32\_RGB
@par
Example : texture mrt\_output target\_width target\_height PF\_FLOAT16\_RGBA PF\_FLOAT16\_RGBA chain\_scope

@note
It is imperative that if you use MRT that the shaders that render to it render to ALL the targets. Not doing so can cause undefined results. It is also important to note that although you can use different pixel formats for each target in a MRT, each one should have the same total bit depth since most cards do not support independent bit depths. If you try to use this feature on cards that do not support the number of MRTs you’ve asked for, the technique will be skipped (so you ought to write a fallback technique).


<a name="compositor_005ftexture_005fref"></a><a name="texture_005fref"></a>

## texture\_ref

This declares a reference of a texture from another compositor to be used in this compositor.

@par
Format: texture\_ref &lt;Local_Name&gt; &lt;Reference_Compositor&gt; &lt;Reference_Texture_Name&gt;

@param Local_Name
A name to give the referenced texture, which must be unique within this compositor. This name is used to reference the texture in @ref Compositor-Target-Passes, when the texture is rendered to, and in @ref Compositor-Passes, when the texture is used as input to a material rendering a fullscreen quad.

@param Reference_Compositor
The name of the compositor that we are referencing a texture from

@param Reference_Texture_Name
The name of the texture in the compositor that we are referencing

Make sure that the texture being referenced is scoped accordingly (either chain or global scope) and placed accordingly during chain creation (if referencing a chain-scoped texture, the compositor must be present in the chain and placed before the compositor referencing it).

@par
Example : texture\_ref GBuffer GBufferCompositor mrt\_output

<a name="compositor_005fscheme"></a><a name="scheme-2"></a>

## scheme

This gives a compositor technique a scheme name, allowing you to manually switch between different techniques for this compositor when instantiated on a viewport by calling Ogre::CompositorInstance::setScheme.

@par
Format: scheme &lt;Name&gt;

<a name="compositor_005flogic"></a><a name="compositor_005flogic-1"></a>

## compositor\_logic

This connects between a compositor and code that it requires in order to function correctly. When an instance of this compositor will be created, the compositor logic will be notified and will have the chance to prepare the compositor’s operation (for example, adding a listener).

@par
Format: compositor\_logic &lt;Name&gt;

Registration of compositor logics is done by name through Ogre::CompositorManager::registerCompositorLogic.

# Target Sections {#Compositor-Target-Passes}

A target section defines the rendering of either a render texture or the final output. You can update the same target multiple times by adding more than one target section to your compositor script - this is very useful for ’ping pong’ renders between a couple of render textures to perform complex convolutions that cannot be done in a single render, such as blurring.

There are two types of target sections, the sort that updates a render texture

@par
Format: target &lt;Name&gt; [&lt;slice&gt;] { }

and the sort that defines the final output render

@par
Format: target\_output { }

The contents of both are identical, the only real difference is that you can only have a single target\_output entry, whilst you can have many target entries. 

Note, the target entry can refer to @ref Cube-map-textures. Therefore, it takes an optional _decimal_ slice parameter that specifies which face you want to render on. The default is 0, hence +X.

Here are the attributes you can use in a ’target’ or ’target\_output’ section of a .compositor script:

-   [input](#compositor_005ftarget_005finput)
-   [only\_initial](#only_005finitial)
-   [visibility\_mask](#visibility_005fmask)
-   [lod\_bias](#compositor_005flod_005fbias)
-   [material_scheme](#material_005fscheme)
-   [shadows](#compositor_005fshadows)

<a name="compositor_005ftarget_005finput"></a><a name="input"></a>

## input

Sets input mode of the target, which tells the target pass what is pulled in before any of its own passes are rendered.

@par
Format: input (none | previous)
@par
Default: input none

@param none
The target will have nothing as input, all the contents of the target must be generated using its own passes. Note this does not mean the target will be empty, just no data will be pulled in. For it to truly be blank you’d need a ’clear’ pass within this target.

@param previous
The target will pull in the previous contents of the viewport. This will be either the original scene if this is the first compositor in the chain, or it will be the output from the previous compositor in the chain if the viewport has multiple compositors enabled.

</dd> </dl> <a name="only_005finitial"></a><a name="only_005finitial-1"></a>

## only\_initial

If set to on, this target pass will only execute once initially after the effect has been enabled. This could be useful to perform once-off renders, after which the static contents are used by the rest of the compositor.

@par
Format: only\_initial (on | off) 
@par
Default: only\_initial off

<a name="visibility_005fmask"></a><a name="visibility_005fmask-1"></a>

## visibility\_mask

Sets the visibility mask for any render\_scene passes performed in this target pass. This is a bitmask (specified as decimal or hex) and maps to Ogre::Viewport::setVisibilityMask.
@par
Format: visibility\_mask &lt;mask&gt;
@par
Default: visibility\_mask 0xFFFFFFFF

<a name="compositor_005flod_005fbias"></a><a name="lod_005fbias"></a>

## lod\_bias

Set the scene LOD bias for any render\_scene passes performed in this target pass. The default is 1.0, everything below that means lower quality, higher means higher quality.
@par
Format: lod\_bias &lt;lodbias&gt;
@par
Default: lod\_bias 1.0

<a name="compositor_005fshadows"></a><a name="shadows"></a>

## shadows

Sets whether shadows should be rendered during any render\_scene pass performed in this target pass.

@par
Format: shadows (on | off)
@par
Default: shadows on

<a name="material_005fscheme"></a><a name="material_005fscheme-1"></a>

## material\_scheme

If set, indicates the material scheme to use for any render\_scene pass. Useful for performing special-case rendering effects.

@par
Format: material\_scheme &lt;scheme name&gt;
@par
Default: None

# Passes {#Compositor-Passes}

A pass is a single rendering action to be performed in a target section.
@par
Format: pass &lt;type&gt; \[custom name\] { }

There are the following types of a pass:

<dl compact="compact">
<dt>clear</dt> <dd>

This kind of pass sets the contents of one or more buffers in the target to a fixed value. So this could clear the colour buffer to a fixed colour, set the depth buffer to a certain set of contents, fill the stencil buffer with a value, or any combination of the above.

</dd> <dt>stencil</dt> <dd>

This kind of pass configures stencil operations for the subsequent passes. It can set the stencil compare function, operations and reference values for you to perform your own stencil effects.

</dd> <dt>render\_scene</dt> <dd>

This kind of pass performs a regular rendering of the scene. It will use the [visibility\_mask](#visibility_005fmask), [lod\_bias](#compositor_005flod_005fbias), and [material\_scheme](#material_005fscheme) from the parent target pass.

</dd> <dt>render\_quad</dt> <dd>

This kind of pass renders a quad over the entire render target, using a given material. You will undoubtedly want to pull in the results of other target passes into this operation to perform fullscreen effects.

</dd> <dt>compute</dt> <dd>

This kind of a pass dispatches a compute shader as attached to the given material. Compute shaders are independent from normal rendering pipeline as triggered by `render_scene` or `render_quad`. They do not have any predefined input/ outputs but rather read/ write to any buffers you attach to them.

</dd> <dt>render\_custom</dt> <dd>

This kind of pass is just a callback to user code for the composition pass specified in the custom name (and registered via Ogre::CompositorManager::registerCustomCompositionPass) and allows the user to create custom render operations for more advanced effects. This is the only pass type that requires the custom name parameter.

</dd> </dl>

## render_quad & compute

Here are the attributes you can use in the @c render_quad and @c compute sections of a .compositor script:

-   [material](#material)
-   [input](#compositor_005fpass_005finput)
-   [identifier](#compositor_005fpass_005fidentifier)
-   [thread_groups](#thread_groups)
-   [quad_normals](#quad_normals)

<a name="material"></a><a name="material-1"></a>

### material

Sets the material to be used. With `compute` passes only the compute shader is used and only global auto parameter can be accessed.
For `render_quad` you will want to use shaders in this material to perform fullscreen effects, and use the [input](#compositor_005fpass_005finput) attribute to map other texture targets into the texture bindings needed by this material. 

@par
Format: material &lt;Name&gt;

<a name="compositor_005fpass_005finput"></a><a name="input-1"></a>

### input

This is how you map one or more local @ref compositor-texture into the material you’re using to render. To bind more than one texture, repeat this attribute with different texUnit indices.

@par
Format: input &lt;texUnit&gt; &lt;name&gt; \[&lt;mrtIndex&gt;\]

@param texUnit
The index of the target texture unit, must be a number in the range \[0, OGRE\_MAX\_TEXTURE\_LAYERS-1\].
@param name
The name of the local render texture to bind, as declared by @ref compositor-texture and rendered to in one or more @ref Compositor-Target-Passes.
@param mrtIndex
If the local texture that you’re referencing is a Multiple Render Target (MRT), this identifies the surface from the MRT that you wish to reference (0 is the first surface, 1 the second etc).

@par
Example: input 0 rt0

<a name="compositor_005fpass_005fidentifier"></a><a name="identifier"></a>

### identifier

Associates a numeric identifier with a pass involving a material. This is useful for registering a listener with Ogre::CompositorInstance::addListener, and being able to identify which pass it is that’s being processed, so that material parameters can be varied. Numbers between 0 and 2^32 - 1 are allowed.

@par
Format: identifier &lt;number&gt; 
@par
Example: identifier 99945
@par
Default: identifier 0

<a name="quad_normals"></a>

### quad_normals

Pass the camera Frustum far corner vectors in the quad normals for passes of type `quad`. This is particularly useful for efficiently reconstructing position using only the depth and the corners.

@par
Format: quad_normals &lt;camera_far_corners_world_space|camera_far_corners_view_space&gt;
@par
Default: None

<a name="thread_groups"></a>

### thread_groups

Passes of type `compute` operate on an abstract "compute space". This space is typically divided into threads and thread groups (work groups). The size of a thread group is defined inside the compute shader itself. This defines how many groups should be launched.

@par
Example: if you want to process a 256x256px image and have a thread group size of 16x16x1, you want to specify `16 16 1` here as well.

@par
Format: thread_groups &lt;groups_x&gt; &lt;groups_y&gt; &lt;groups_z&gt;

## render_scene

Here are the attributes you can use in a @c render_scene section of a .compositor script:

-   [first\_render\_queue](#first_005frender_005fqueue)
-   [last\_render\_queue](#last_005frender_005fqueue)
-   [material\_scheme](#compositor_005fpass_005fmaterial_005fscheme)
-   [camera](#camera)

<a name="first_005frender_005fqueue"></a><a name="first_005frender_005fqueue-1"></a>

### first\_render\_queue

For passes of type ’render\_scene’, this sets the first render queue id that is included in the render. Defaults to the value of Ogre::RENDER_QUEUE_BACKGROUND.
@par
Format: first\_render\_queue &lt;id&gt;
@par
Default: first\_render\_queue 0

<a name="last_005frender_005fqueue"></a><a name="last_005frender_005fqueue-1"></a>

### last\_render\_queue

For passes of type ’render\_scene’, this sets the last render queue id that is included in the render. Defaults to the value of Ogre::RENDER_QUEUE_SKIES_LATE.
@par
Format: last\_render\_queue &lt;id&gt;
@par
Default: last\_render\_queue 95

<a name="compositor_005fpass_005fmaterial_005fscheme">

### material\_scheme

If set, indicates the material scheme to use for this pass only. Useful for performing special-case rendering effects. This will overwrite any scheme set in the parent @ref Compositor-Target-Passes.
@par
Format: material\_scheme &lt;scheme name&gt;
@par
Default: None

<a name="camera"></a>

### camera

Use a camera different from the output Viewport for rendering the scene into this target. Very useful for reflection effects like mirrors or water. The camera will be searched by name in the currently active scene and must be created prior to activating the compositor.

The optional parameter @c align_to_face automatically rotates the camera towards the target cubemap face, when rendering to a cubic texture.

@par
Format: camera &lt;name&gt; [&lt;align_to_face&gt;]
@par
Default: None

## clear {#Clear-Section}

For passes of type ’clear’, this section defines the buffer clearing parameters.  

@par
Format: pass clear { }

Here are the attributes you can use in a ’clear’ section of a .compositor script:

-   [buffers](#compositor_005fclear_005fbuffers)
-   [colour\_value](#compositor_005fclear_005fcolour_005fvalue)
-   [depth\_value](#compositor_005fclear_005fdepth_005fvalue)
-   [stencil\_value](#compositor_005fclear_005fstencil_005fvalue) <a name="compositor_005fclear_005fbuffers"></a><a name="buffers"></a>

    ### buffers

    Sets the buffers cleared by this pass.

    @par
    Format: buffers \[colour\] \[depth\] \[stencil\] 
    @par
    Default: buffers colour depth

    <a name="compositor_005fclear_005fcolour_005fvalue"></a><a name="colour_005fvalue"></a>

    ### colour\_value

    Set the colour used to fill the colour buffer by this pass, if the colour buffer is being cleared
    @par
    Format: colour\_value (&lt;red&gt; &lt;green&gt; &lt;blue&gt; &lt;alpha&gt; | auto)
    @par
    Default: colour\_value 0 0 0 0

    If set to `auto` the background colour of the viewport is used, to which the parent compositor is attached to.

    <a name="compositor_005fclear_005fdepth_005fvalue"></a><a name="depth_005fvalue"></a>

    ### depth\_value

    Set the depth value used to fill the depth buffer by this pass, if the depth buffer is being cleared
    @par
    Format: depth\_value &lt;depth&gt; 
    @par
    Default: depth\_value 1.0

    <a name="compositor_005fclear_005fstencil_005fvalue"></a><a name="stencil_005fvalue"></a>

    ### stencil\_value

    Set the stencil value used to fill the stencil buffer by this pass, if the stencil buffer is being cleared
    @par
    Format: stencil\_value &lt;value&gt; 
    @par
    Default: stencil\_value 0.0

## stencil {#Stencil-Section}

For passes of type ’stencil’, this section defines the stencil operation parameters. 

@par
Format: pass stencil { }

@copydetails Ogre::StencilState

Here are the attributes you can use in a ’stencil’ section of a .compositor script:

-   [check](#compositor_005fstencil_005fcheck)
-   [comp\_func](#compositor_005fstencil_005fcomp_005ffunc)
-   [ref\_value](#compositor_005fstencil_005fref_005fvalue)
-   [mask](#compositor_005fstencil_005fmask)
-   [fail\_op](#compositor_005fstencil_005ffail_005fop)
-   [depth\_fail\_op](#compositor_005fstencil_005fdepth_005ffail_005fop)
-   [pass\_op](#compositor_005fstencil_005fpass_005fop)
-   [two\_sided](#compositor_005fstencil_005ftwo_005fsided) <a name="compositor_005fstencil_005fcheck"></a><a name="check"></a>

    ### check

    Enables or disables the stencil check. The rest of the options in this section do nothing if the stencil check is off.
    @par
    Format: check (on | off)

    <a name="compositor_005fstencil_005fcomp_005ffunc"></a><a name="comp_005ffunc"></a>

    ### comp\_func

    @copybrief Ogre::StencilState::compareOp

    @par
    Format: comp\_func &lt;func&gt;
    @par
    Default: comp\_func always\_pass

    @param func one of Ogre::CompareFunction without the `CMPF_` prefix. E.g. `CMPF_LESS_EQUAL` becomes `less_equal`.

    <a name="compositor_005fstencil_005fref_005fvalue"></a><a name="ref_005fvalue"></a>

    ### ref\_value

    @copybrief Ogre::StencilState::referenceValue

    @par
    Format: ref\_value &lt;value&gt; 
    @par
    Default: ref\_value 0.0

    <a name="compositor_005fstencil_005fmask"></a><a name="mask"></a>

    ### mask

    @copybrief Ogre::StencilState::compareMask

    @par
    Format: mask &lt;value&gt; 
    @par
    Default: mask 4294967295

    <a name="compositor_005fstencil_005ffail_005fop"></a><a name="fail_005fop"></a>

    ### fail\_op

    @copybrief Ogre::StencilState::stencilFailOp

    @par
    Format: fail\_op &lt;op&gt;
    @par
    Default: fail\_op keep

    @param op one of Ogre::StencilOperation without the `SOP_` prefix. E.g. `SOP_INCREMENT_WRAP` becomes `increment_wrap`.

    <a name="compositor_005fstencil_005fdepth_005ffail_005fop"></a><a name="depth_005ffail_005fop"></a>

    ### depth\_fail\_op

    @copybrief Ogre::StencilState::depthFailOp

    @par
    Format: depth\_fail\_op &lt;op&gt;
    @par
    Default: depth\_fail\_op keep

    @param op one of Ogre::StencilOperation without the `SOP_` prefix. E.g. `SOP_INCREMENT_WRAP` becomes `increment_wrap`.

    <a name="compositor_005fstencil_005fpass_005fop"></a><a name="pass_005fop"></a>

    ### pass\_op

    @copybrief Ogre::StencilState::depthStencilPassOp

    @par
    Format: pass\_op &lt;op&gt;
    @par
    Default: pass\_op keep

    @param op one of Ogre::StencilOperation without the `SOP_` prefix. E.g. `SOP_INCREMENT_WRAP` becomes `increment_wrap`.

    <a name="compositor_005fstencil_005ftwo_005fsided"></a><a name="two_005fsided"></a>

    ### two\_sided

    @copybrief Ogre::StencilState::twoSidedOperation

    @par
    Format: two\_sided (on | off)
    @par
    Default: two\_sided off


# Applying a Compositor {#Applying-a-Compositor}

Adding a compositor instance to a viewport is very simple. All you need to do is:

```cpp
Ogre::CompositorManager::getSingleton().addCompositor(viewport, compositorName);
```



Where viewport is a pointer to your viewport, and compositorName is the name of the compositor to create an instance of. By doing this, a new instance of a compositor will be added to a new compositor chain on that viewport. You can call the method multiple times to add further compositors to the chain on this viewport. By default, each compositor which is added is disabled, but you can change this state by calling:

```cpp
Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport, compositorName, enabledOrDisabled);
```

For more information on defining and using compositors, see @c Sample_Compositor in the Samples area, together with the Examples.compositor script in the media area.

# Programmatic creation {#Compositor-API}

In case you need to create Compositors programmatically, see the following example for how the script is mapped to the API.

@snippet Samples/Media/materials/scripts/Examples.compositor glass_script

becomes
```cpp
using namespace Ogre;
CompositorPtr glass = CompositorManager::getSingleton().create("Glass", RGN_DEFAULT);

CompositionTechnique *t = glass->createTechnique();
auto td = t->createTextureDefinition("rt0");
td->width = 0;
td->height = 0;
td->format = PF_BYTE_RGB;

CompositionTargetPass *tp = t->createTargetPass();
tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
tp->setOutputName("rt0");

CompositionTargetPass *tp = t->getOutputTargetPass();
tp->setInputMode(CompositionTargetPass::IM_NONE);

CompositionPass *pass = tp->createPass(CompositionPass::PT_RENDERQUAD)
pass->setMaterialName("Ogre/Compositor/GlassPass");
pass->setInput(0, "rt0");
```