/******************************************************************************\
* UsersGuide.hpp                                                               *
* Additional Doxygen documentation for Diluculum: user's guide.                *
*                                                                              *
*                                                                              *
* Copyright (C) 2005-2006 by Leandro Motta Barros.                             *
*                                                                              *
* Permission is hereby granted, free of charge, to any person obtaining a copy *
* of this software and associated documentation files (the "Software"), to     *
* deal in the Software without restriction, including without limitation the   *
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or  *
* sell copies of the Software, and to permit persons to whom the Software is   *
* furnished to do so, subject to the following conditions:                     *
*                                                                              *
* The above copyright notice and this permission notice shall be included in   *
* all copies or substantial portions of the Software.                          *
*                                                                              *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR   *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS *
* IN THE SOFTWARE.                                                             *
\******************************************************************************/


/** @page pag-UsersGuide Diluculum's Quick User's Guide

\section Introduction

Diluculum is a library that intends to make the coexistence of C++ and Lua more
harmonious. In this regard, it offers essentially three capabilities:

- A nice way to access data stored in a Lua interpreter. This is provided by the
  \c Diluculum::LuaState class, which encapsulates a <tt>lua_State*</tt>.

- A not so bad way to write functions in C++ and make them available to
  Lua. This is done with the use of macros like \c DILUCULUM_WRAP_FUNCTION().

- A limited and just barely usable way to register C++ classes and objects in a
  Lua state. This is done with a set of macros, like \c DILUCULUM_BEGIN_CLASS()
  and \c DILUCULUM_REGISTER_OBJECT().

The next sections discuss each of these three items in more detail. One more
note: Diluculum has unit tests for all its features, at the \c Tests directory
in the source distribution. These tests also serve good as examples on how to
use the library.



\section sec-AcessingLuaState Accessing Data In a Lua State

The class \c Diluculum::LuaState encapsulates a <tt>lua_State*</tt>, that is a
Lua interpreter. Lua is frequently used as a configuration language, and for
this purpose, a \c Diluculum::LuaState works pretty well. Suppose we have a Lua
file named \c config.lua which will be used as the configuration file for some
application:

\verbatim
-- A configuration file

FavoriteColor = "blue"

FavoritePiApproximation = 22/7

UserInfo = {
   Name = "Fulano de Tal",
   Age = 33,
   LikesLargeIcons = true,
}

WindowSize = { 456, 234 }

function EmphasizeFunc(s)
   return "<EMPH>"..s.."</EMPH>"
end

\endverbatim

In order to access the information in this file from C++, the first step is to
create a \c Diluculum::LuaState, and execute the file, as show below.

\code
#include <Diluculum/LuaState.hpp>

// ...

Diluculum::LuaState ls;
ls.doFile ("config.lua");
\endcode

Then, you are ready to used the subscript operator of \c Diluculum::LuaState to
access the variables living in it.

\code
const std::string favColor = ls["FavoriteColor"].value().asString();
const double favPiApprox = ls["FavoritePiApproximation"].value().asNumber();

const std::string userName = ls["UserInfo"]["Name"].value().asString();
const int userAge = static_cast<int>(ls["UserInfo"]["Age"].value().asNumber());
const bool userLikesLargeIcons = ls["UserInfo"]["LikesLargeIcons"].value().asBoolean();

const int winWidth = static_cast<int>(ls["WindowSize"][1].value().asNumber());
const int winHeight = static_cast<int>(ls["WindowSize"][2].value().asNumber());
\endcode

These calls to \c value(), \c asNumber(), \c asString() and so on are a little
cumbersome, but not \e that bad. Anyway, if you are just testing for equality
(or inequality), you can omit them:

\code
if (ls["UserInfo"]["LikesLargeIcons"] == true)
{
   // ...
}

if (ls["FavoriteColor"] != "blue")
{
   std::cout << "You have bad taste for colors!\n";
}
\endcode

You may also have noticed that a function was defined in \c config.lua. It can
be called from C++ with relative ease. The only thing to notice is that Lua
functions can return an arbitrary number of values, so the return in C++ will
be a \c Diluculum::LuaValueList. The code snippet below shows how to call that
function.

\code
Diluculum::LuaValueList ret = ls["EmphasizeFunc"]("String to be emphasized");
std::cout << ret[0].asString() << '\n';
\endcode

Notice that no call to \c value() is necessary here, because function calls will
aready return values (<tt>Diluculum::LuaValue</tt>s) instead of variables
(<tt>Diluculum::LuaVariable</tt>s).

The subscript operator also provides write access to the Lua state, so you can
change or add new variables. Perhaps this doesn't make much sense if you are
using Lua just as a simple configuration language, as shown up to this point,
but the feature is useful nevertheless. The example below shows how to do this.

\code
ls["newNumber"] = -123.456;
ls["newString"] = "Ahhhh!";
ls["FavoritePiApproximation"] = 3.14159265;
\endcode



\section sec-WrappingFunctions Wrapping C++ Functions

First of all, if you happen to have a <tt>lua_CFunction</tt> (the functions used
with the Lua API), you can just assign them to a \c Diluculum::LuaState, as done
in the previous section, and as shown below.

\code
int aLuaCFunction (lua_State* ls)
{
   // ...
}

// ...

// Create a Lua state and register the function there
Diluculum::LuaState ls;
ls["Func"] = aLuaCFunction;

// Call the function from Lua
ls.doString ("Func (1, 2, 3, 'four')");
\endcode

It turns out that writing a <tt>lua_CFunction</tt> involves manipulating a raw
<tt>lua_State*</tt>, which may not be what you want. So, Diluculum can
automatically create a <tt>lua_CFunction</tt> from a function that takes and
returns a \c Diluculum::LuaValueList. Then, this function can be registered in a
\c Diluculum::LuaState as above. The code snippet below demonstrates this.

\code
#include <Diluculum/LuaWrappers.hpp>

// [...]

// In Lua, this will take one number parameter and return three numbers, which
// are equal the parameter, two times the parameter, and three times the
// parameter. Not very useful, uh?
Diluculum::LuaValueList MyFunction (const Diluculum::LuaValueList& params)
{
   if (params.size() != 1 || params[0].type() != LUA_TNUMBER)
      throw Diluculum::LuaError ("Bad parameters!");

   Diluculum::LuaValueList ret;

   ret.push_back (params[0].asNumber());
   ret.push_back (params[0].asNumber() * 2);
   ret.push_back (params[0].asNumber() * 3);

   return ret;
}

// Create a 'lua_CFunction'
DILUCULUM_WRAP_FUNCTION (MyFunction);

// ...

// Create a Lua state and register the function.
Diluculum::LuaState ls;
ls["MyFunction"] = DILUCULUM_WRAPPER_FUNCTION (MyFunction);

// Now, 'MyFunction' can be called from Lua
ls.doString ("a, b, c = MyFunction (4.5)");
\endcode

Perhaps the only important thingh to notice in the example above is that
throwing a \c Diluculum::LuaError is the proper way to report an error from the
wrapped function. For those that know the Lua C API, this exception will be
handled and "translated" to a call to \c lua_error().

And that's it. Not exactly a proper and "generic" wrapper like
<A href="http://www.tecgraf.puc-rio.br/~celes/tolua/">toLua</A>,
<A href="http://www.codenix.com/~tolua/">toLua++</A> or
<A href="http://www.rasterbar.com/products/luabind.html">Luabind++</A>,
but may be useful for someone (it is for me!)



\section sec-WrappingClasses Wrapping C++ Classes and Objects

Finally, Diluculum can has limited support for wrapping C++ classes and objects
to Lua and registering them in a Lua state. I call it "limited" because it
doesn't support real object orientation (most notably, it lacks inheritance). It
is also somewhat cumbersome to use. But, again, it may be useful for someone
other than me.

Diluculum cannot handle any class. Just as it expected functions with a specific
signature (see the previous section), classes must follow some rules in order to
be usable with Diluculum. First, it must have a constructor that takes a
\c Diluculum::LuaValueList as parameter. Second, all methods that will be
exported must take a \c Diluculum::LuaValueList as parameter and also return a
\c Diluculum::LuaValueList. The class below is an example of a "valid" class.

\code
// A class that stores one value. Duh.
class ValueBox
{
   public:
      // The constructor taking a 'Diluculum::LuaValueList' parameter.
      ValueBox (const Diluculum::LuaValueList& params)
      {
         if (params.size() > 0)
            value_ = params[0];
      }

      // Stores the value passed as parameter in the box, and returns the value
      // previously stored there.
      Diluculum::LuaValueList swap (const Diluculum::LuaValueList& params)
      {
         if (params.size() != 1)
            throw Diluculum::LuaError ("Exactly one parameter was expected.");

         Diluculum::LuaValueList ret;
         ret.push_back (value_);
         value_ = params[0];
         return ret;
      }

   private:
      Diluculum::LuaValue value_;
};
\endcode

As was the case with free functions, throwing a \c Diluculum::LuaError is the
way to report errors from methods. The first step to make this class available
to a Lua state is to call the \c DILUCULUM_WRAP_METHOD() macro for every
exported method. This macro will create a function wrapping the method. In this
case, we have just one exported method:

\code
DILUCULUM_WRAP_METHOD (ValueBox, swap);
\endcode

The next step is to call a sequence of macros that will create some data
structures representing the class. (For the Lua-inclined readers: more
specifically, it will create a global \c Diluculum::LuaValueMap representing the
class; it will be used as the metatable for the objects in Lua.) These macro
calls are shown below.

\code
DILUCULUM_BEGIN_CLASS (ValueBox);
   DILUCULUM_CLASS_METHOD (ValueBox, swap);
DILUCULUM_END_CLASS (ValueBox);
\endcode

Notice that it was necessary to make another macro call for every exported
method. Ugly, isn't it? Anyway, now we are ready to register this class in a
Lua state, so that it can actually be use. This requires one more macro call.
After this final macro call, objects of this class can be instantiated from Lua.
The code snippet below shows how to do this.

\code
Diluculum::LuaState ls;
DILUCULUM_REGISTER_CLASS (ls, ValueBox);

ls.doString ("box = ValueBox.new(3)");
ls.doString ("print (box:swap('foo'))"); // prints '3'
ls.doString ("print (box:swap(789.987))"); // prints 'foo'

\endcode


\subsection sec-MemoryManagement Memory Management

When an object instantiated in Lua is garbage collected, the object's destructor
will be properly called, so that its resources are properly freed. It is also
possible to explicitly call the object's destructor in order to free its
resources before the object is garbage collected. See below.

\code
Diluculum::LuaState ls;
DILUCULUM_REGISTER_CLASS (ls, ValueBox);

ls.doString ("box = ValueBox.new()");

// ...use 'box'...

ls.doString ("box:delete()");
\endcode

And this is a good moment to discuss a very important case in which calling the
destructor explicitly in Lua is a good idea. The problem is that Lua doesn't
know how much memory an object occupies. For every object instantiated using
Diluculum's facilities, Lua is only aware of a little of memory (a pointer and
some additional flags). If the object itself allocates more memory (as any
decent object is supposed to do), Lua will not know this, and the garbage
collector will not detect the "right" amount of memory allocated from within
Lua. So, if you instantiate a big object, you better free it manually from Lua
as soon as you don't need it anymore. Likewise, if you allocate many objects (in
a loop, perhaps), you better delete each of them manually.


\subsection sec-RegisteringObjects Registering Objects

Sometimes one may have an object constructed in the C++, and want to make it
available to a Lua state. In other words, one may want to create an object in
C++ and call its methods from Lua. This can be done with Diluculum, using yet
another macro, as shown below.

\code
Diluculum::LuaState ls;

Diluculum::LuaValueList params;
ValueBox vb (params);

DILUCULUM_REGISTER_OBJECT (ls["box"], ValueBox, vb);

ls.doString ("box:swap (123)");

std::cout << vb.swap("abc") << '\n'; // prints '123'.
\endcode

Notice that when \c box is garbage collected in Lua, the C++ object will \e not
be destroyed. In this case, cleaning the object is responsibility of the
programmer on the C++ side.

*/