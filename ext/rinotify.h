/*****************************************************
 * copyright (C) 2007 by Rob Merrell
 * rob@migrob.com
 *
 * ***************************************************/

#ifndef RINOTIFY_H
#define RINOTIFY_H

#include "ruby.h"
#include <sys/inotify.h>

#define CURRENT_VERSION "0.9"
#define EVENT_SIZE sizeof(struct inotify_event)
#define BUFFER_SIZE 16384

// RInotify class
static VALUE rb_cRInotify;

/* Non Ruby-implimented Prototypes */

// declare the inotify events as constants
static void rinotify_declare_events(VALUE);

// declare any instance variables we will need
static void rinotify_declare_instance_vars(VALUE);



/* Ruby Prototypes */

/*
 * call-seq:
 *	RInotify.new -> RInotify
 *
 *	Returns a new RInotify object.
 *
 *	Example Usage:
 *	@rinotify = RInotify.new
 */
static VALUE rb_rinotify_new(VALUE);


/*
 * call-seq:
 *	RInotify.version -> String
 *
 *	Returns the current version.
 *
 *	Example Usage:
 *	@rinotify.version => "0.1.0"
 */
static VALUE rb_rinotify_version(VALUE);


/*
 * call-seq:
 *		RInotify.close -> nil
 *
 *	Clean up and close inotify.
 *
 *	Example Usage:
 *	@rinotify.close
 */
static VALUE rb_rinotify_close(VALUE);


/*
 * call-seq:
 *	RInotify.add_watch(filename, event masks) -> watch descriptor
 *
 *	Adds a watch on a file or directory.
 *
 *	The event masks are simple a bitmask and should be joined like:
 *	RInotify::OPEN | RInotify::MODIFY
 *
 *	Example Usage:
 *	@rinotify = RInotify.new
 *	@rinotify.add_watch("/home/rob/Desktop/my_file.txt", RInotify::MODIFY | RInotify::DELETE_SELF | RInotify::OPEN)
 */
static VALUE rb_rinotify_add_watch(VALUE, VALUE, VALUE);


/*
 * call-seq:
 *	RInotify.rm_watch(watch descriptor) -> closed watch descriptor
 *
 *	Remove a watch descriptor.  Note that this is not a necessary step
 *	when closing inotify.
 *
 *	Example Usage:
 *	@rinotify = RInotify.new
 *	wd = @rinotify.add_watch("/home/rob/Desktop/my_file.txt", RInotify::MODIFY | RInotify::DELETE_SELF | RInotify::OPEN)
 *	@rinotify.rm_watch(wd)
 */
static VALUE rb_rinotify_rm_watch(VALUE, VALUE);


/*
 * call-seq:
 *	RInotify.wait_for_events(seconds to time out) -> true or false 
 *
 *	Waits for events to be received from inotify.
 *	Returns true when there are events waiting in the queue
 *	and false if not.
 *
 *	Example Usage:
 *	@rinotify = RInotify.new
 *	@rinotify.add_watch("/home/rob/Desktop/my_file.txt", RInotify::MODIFY | RInotify::DELETE_SELF | RInotify::OPEN)
 *
 *	# set time out at 5 seconds
 *	has_events = @rinotify.wait_for_events(5)
 */
static VALUE rb_rinotify_wait_for_events(VALUE, VALUE);


/*
 * call-seq:
 *	RInotify.each_event -> RInotifyEvent
 *
 *	Yields an RInotifyEvent object in a block
 *
 *	Example Usage:
 *	has_events = @rinotify.wait_for_events(5)
 *	if has_events
 *		@rinotify.each_event {|revent|
 *			...
 *		}
 *	end
 */
static VALUE rb_rinotify_each_event(VALUE);


/*
 * call-seq:
 *	RInotify.watch_descriptors -> Hash(watch descriptor, file name)
 *
 *	Returns a hash of all watch_descriptors and file name of each file or directory
 *	being watched.
 *
 *	Example Usage:
 *	@rinotify.watch_descriptors => [1, "/home/rob/Desktop/my_file.txt"]
 */
static VALUE rb_rinotify_watch_descriptors(VALUE);


/*
 * call-seq:
 *	RInotify.event_queue_size -> Fixnum
 *
 *	Returns the current size of the event queue.  This is useful if you want to throttle
 *	your event checks
 *
 *	Example Usage:
 *	while (queue_size < 128)
 *		if @rinotify.wait_for_events(5)
 *			queue_size = @rinotify.event_queue_size
 *		end
 *	end	
 */
static VALUE rb_rinotify_queue_size(VALUE);

#endif
