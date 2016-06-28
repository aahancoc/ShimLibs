The ShimLibs
---

The ShimLibs are a collection of hacky functions designed to make using functions from various libraries just a little bit more bearable. You probably *shouldn't* use them, but you could.

##Installation
You don't. (Things like AlertMsg() and CURRERROR would break if this were a DLL or .so). Instead, just dump this into your project's tree and include the .c files into your Makefile and the .h files into your code.

If your code doesn't use a particular library, just don't include it's files. Everything else *should* adjust its featureset accordingly.

##Notes
* Just about every string output is a malloc()-allocated string. This means you should free it when you're done.
* This uses asprintf() extensively. If you don't have asprintf(), get a compiler that does. (Or just use an asprintf() shim, if you have no choice.)
* There is a global variable named CURRERROR that the ShimLibs dump custom errors to. Don't forget to a.) define CURRERROR and b.) check it after calling any 

##Features
###Jansson
There's a couple neat utilities for Jansson, the JSON C library. There's one for safe loading with error handling (or, well, error display) and a couple for getting a number/string from a value of any type.

###SQLite3
This one is a bit more useful. More framework-y.

As you (hopefully) know, the proper way to run an SQLite query to to pepare, bind, step, and finalize. As you probably also know, this is a rather unweildy pain, especially if you want to check for errors or get output. With the ShimLibs, it's *still* an unweildy pain, but it's slightly more bearable.

Your "silver bullet" here is going to be `SQL_HandleErrors()`. Here's an example of code that uses this.

```cpp
	sqlite3_stmt *command;
	const char *query = "INSERT INTO Users ('Name', 'ID') VALUES (?, ?);";

	if(SQL_HandleErrors(
		__LINE__, sqlite3_prepare_v2(db, query, -1, &command, NULL)
	  ) != 0 || SQL_HandleErrors(
		__LINE__, sqlite3_bind_text(command, 1, user.name, -1. SQLITE_TRANSIENT) |
		          sqlite3_bind_int(command, 2, user.id)
	  ) != 0 || SQL_HandleErrors(
		__LINE__, sqlite3_step(command)
	  ) != 0 || SQL_HandleErrors(
		__LINE__, sqlite3_finalize(command)
	  ) != 0
	){
		CURRERROR = errCRIT_DBASE;
		free(user);
		return FALSE;
	}
```

What's going on here is that every SQLite call is wrapped in an `SQL_HandleErrors` call. Each call has a line number associated, for debugging purposes.

What `SQL_HandleErrors` does is (among other things) returns 0 if the result code is a non-error (such as `OK`, `DONE` or `RECORD` and return -1 otherwise. (It could also return the error. There's not much of a reson to go one way or the other.)

The if() statement executes each `SQL_HandleErrors` call. If it returns 0, it executes the next call. If it fails, the or (||) short-circuting kicks in and immediately jumps to the error condition, which deallocates memory and returns. It's a cool hack that has proven very useful compared to the alternatives of either gotos or code duplication.

There are also functions to, immediately after the bind() calls, get the first number, string, or blob. To use these, remove step() and finalize() from the if() statement. After the if() statement, call`SQL_GetWhatever()` and then finalize(). They're nice if all you need is a number/string/...

There's also a function that returns the results table as a JSON object, for easy analysis using Jansson's for loop macros or object indexing. It might seem wasteful, but it is ungodly convienent compared to doing things the 'proper' way.

##Win32
This is where get get flat-out *hacky*. You thought SQLite was weird? Hah!

First thing, there's functions designed to select a folder or a file via GUI and return it's path. It's just a boilerplate wrapper, really. Also, I doubt you care, but `SelectFile()` doesn't run on Windows NT 4.0. I don't know why.

There's also some functions that display a progress dialog, updatable via simple commands. Embed this into a window, or don't. I don't care. The progress dialog is loaded via a resource, so make sure you have the ability to use that.

There's also a function that makes scrolling... possible. Say you want to scroll a window. You would think you could use the ScrollWindow() function.  As it turns out, you can't. You need to provide this yourself. 

We do this by simply moving each window within the window down by however much you've scrolled by. Stupidly elegant. See the included `ScrollByMove-example.c` for a sample window procedure that implements scrolling and vertical resizing.

And now here's the big hack. Say you want a dialog window to be readable whether or not the user is using a large font or a small font. (Or, in today's terminology, you want a DPI-aware window.) If you know anything about dialogs, you have to declare a font and a font size. This font then defines the size and position of everything else in relation to it.  Problem is, that font size is *the* font size, regardless of what the user chooses in the system settings. On a 4k monitor, either you have a blurry upscaled dialog or a dialog the size of a postage stamp. Neither is good.

To circumvent this, we intercept the stored dialog resource, alter the font size, and then load it as usual. To do this, we have to traverse the dialog resource byte by byte until we reach the font and then swap out the size number. I have the feeling that nobody else ever did this and it would make your program look hideously out of place, but it is a very useful feature to have.

That being said, any developer today would be using .NET or WPF something, so the point is moot.

##AlertMsg
In both the Win32 shim and in a seperate file there is an `AlertMsg()` function. This is used by the ShimLibs to output errors and other stuff. On Windows, this appears as a pop-up messagebox. On not Windows, this is printed to stdout.

If you wish to provide your own version of AlertMsg(), define `SHIMLIBS_PROVIDES_ALERTMSG` in your code and create a AlertMsg function that matches the given function prototype.

##CURRERROR
To handle errors and display sensical messages, the ShimLibs use a global variable named CURRERROR. This is much like the standard errno, except with more customized messages.

You should check the value of CURRERROR after each and every ShimLib call, as the value is cleared upon entering each function.

To handle these errors, there is a function named `ErrCracker()`. These provide generic messages to use to alert the user as to what happened. This function by default clears the error on non-critical errors. If you need to extend or change the behavior of this function, provide your own replacement and use that instead.

There is also a function to convert standard errno errors to CURRERROR errors, to simplify error handling.

Keep your error handling flow in check in a way where you can easily recover from an error if possible, and you quit if you can't. On large codebases this can get very complicated very fast, so watch out. (One trick is to, on a critical error, use a `goto` to skip the rest of the code, cleanup and return to a higher up function that *can* handle it. This is actually something the Linux kernel does, so don't worry.)

---

Hopefully this horiffic hodgepodge of shims is useful for somebody. It's useful for me, at least.
