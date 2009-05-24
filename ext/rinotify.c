/*****************************************************
 * copyright (C) 2007 by Rob Merrell
 * rob@migrob.com
 *
 * ***************************************************/

#include "ruby.h"
#include "rinotify.h"
#include "rinotify_event.h"

#include <sys/inotify.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>

// extension entry point
void Init_rinotify() {
	rb_cRInotify = rb_define_class("RInotify", rb_cObject);	
	rb_cRInotifyEvent = rb_define_class("RInotifyEvent", rb_cObject);
	
	// initialize all of the events
	rinotify_declare_events(rb_cRInotify);		


	// RInotify.new
	rb_define_alloc_func(rb_cRInotify, rb_rinotify_new);
	
	// RInotify.version
	rb_define_method(rb_cRInotify, "version", rb_rinotify_version, 0);

	// RInotify.close
	rb_define_method(rb_cRInotify, "close", rb_rinotify_close, 0);

	// RInotify.add_watch
	rb_define_method(rb_cRInotify, "add_watch", rb_rinotify_add_watch, 2);

	// RInotify.rm_watch
	rb_define_method(rb_cRInotify, "rm_watch", rb_rinotify_rm_watch, 1);

	// RInotify.wait_for_events
	rb_define_method(rb_cRInotify, "wait_for_events", rb_rinotify_wait_for_events, 1);

	// RInotify.each_event
	rb_define_method(rb_cRInotify, "each_event", rb_rinotify_each_event, 0);
	
	// RInotify.watch_descriptors
	rb_define_method(rb_cRInotify, "watch_descriptors", rb_rinotify_watch_descriptors, 0);

	// RInotify.event_queue_size
	rb_define_method(rb_cRInotify, "event_queue_size", rb_rinotify_queue_size, 0);


	/* The following methods are implemented in rinotify_event.c */
	
	// RInotifyEvent.name
	rb_define_method(rb_cRInotifyEvent, "name", rb_rinotify_event_name, 0);
	
	// RInotifyEvent.watch_descriptor
	rb_define_method(rb_cRInotifyEvent, "watch_descriptor", rb_rinotify_event_watch_descriptor, 0);
	
	// RInotifyEvent.check_mask
	rb_define_method(rb_cRInotifyEvent, "check_mask", rb_rinotify_event_check_mask, 1);
}


static VALUE rb_rinotify_new(VALUE klass) {
	VALUE initialized_class;
	
	// initialize inotify
	int *inotify = NULL;
	inotify = malloc(sizeof(int));
	*inotify = inotify_init();

	if (*inotify < 0)
		rb_sys_fail("inotify_init");	
	
	// make sure free is called because we malloc'd above
	initialized_class = Data_Wrap_Struct(klass, NULL, free, inotify);
	
	// initialize all of the instance variables for this class
	rinotify_declare_instance_vars(initialized_class);

	return initialized_class;
}


static VALUE rb_rinotify_version(VALUE self) {
	return rb_str_new2(CURRENT_VERSION);	
}


static VALUE rb_rinotify_close(VALUE self) {
	int *inotify = NULL, close_return;
	Data_Get_Struct(self, int, inotify);

	// close and clean up inotify
	close_return = close(*inotify);
	if (close_return)
		rb_sys_fail("close");	

	return Qnil;
}


static VALUE rb_rinotify_add_watch(VALUE self, VALUE filename, VALUE event_masks) {
	int *inotify = NULL, watch_desc;
	VALUE watch_desc_id, watch_descriptor_list;
	Data_Get_Struct(self, int, inotify);

	// add the watch
	watch_desc = inotify_add_watch(*inotify, RSTRING_PTR(filename), NUM2INT(event_masks));	
	if (watch_desc < 0)
		rb_sys_fail("add_watch");
	
	watch_desc_id = INT2NUM(watch_desc);

	// add the watch descriptor to our list
	watch_descriptor_list = rb_iv_get(self, "@watch_descriptors");
	rb_hash_aset(watch_descriptor_list, watch_desc_id, filename);

	return watch_desc_id;
}


static VALUE rb_rinotify_rm_watch(VALUE self, VALUE watch_desc) {
	int *inotify = NULL, rm_return;
	Data_Get_Struct(self, int, inotify);

	// remove the watch
	rm_return = inotify_rm_watch(*inotify, NUM2INT(watch_desc));
	if (rm_return < 0)
		rb_sys_fail("rm_watch");

	return INT2NUM(rm_return);
}


static VALUE rb_rinotify_wait_for_events(VALUE self, VALUE time_value) {
	struct timeval time;
	fd_set rfds;
	int select_ret, *inotify = NULL;	

	Data_Get_Struct(self, int, inotify);

	// set the timout value
	time.tv_sec = NUM2INT(time_value);
	time.tv_usec = 0;

	// add inotify to the file descriptor set
	FD_ZERO(&rfds);
	FD_SET(*inotify, &rfds);	

	select_ret = rb_thread_select(*inotify + 1, &rfds, NULL, NULL, &time);
	
	if (select_ret < 0)
		rb_sys_fail("select");

	// no events are available and we have timed out
	else if (!select_ret)
		return Qfalse;

	// events are available
	else if (FD_ISSET(*inotify, &rfds))
		return Qtrue;

	// to keep the compiler happy...
	return Qfalse;
}


static VALUE rb_rinotify_each_event(VALUE self) {
	struct inotify_event *event = NULL, *tmp_event = NULL;
	int *inotify = NULL, i = 0, len;
	char buffer[BUFFER_SIZE];

	Data_Get_Struct(self, int, inotify);

	len = read(*inotify, buffer, BUFFER_SIZE);	

	// read each event
	while (i < len) {
		tmp_event = (struct inotify_event *) &buffer[i];

		// copy the tmp_event into our malloc'd event so that it doesn't
		// go out of scope after we yield the object
        event = malloc(EVENT_SIZE + tmp_event->len);
		memmove(event, tmp_event, EVENT_SIZE + tmp_event->len); 

		// construct the RInotifyEvent object
		rb_yield(rb_rinotify_event_new(event));

		i += EVENT_SIZE + event->len;
	}

	return Qnil;
}


static VALUE rb_rinotify_watch_descriptors(VALUE self) {
	return rb_iv_get(self, "@watch_descriptors");
}


static VALUE rb_rinotify_queue_size(VALUE self) {
	int *inotify = NULL, return_val;
	unsigned int queue_size;

	Data_Get_Struct(self, int, inotify);

	// get the queue size
	return_val = ioctl(*inotify, FIONREAD, &queue_size);

	if (return_val < 0)
		rb_sys_fail("event_queue_size");

	return UINT2NUM(queue_size);
}


static void rinotify_declare_events(VALUE klass) {
	// watch events
	rb_const_set(klass, rb_intern("IN_ACCESS"), INT2NUM(IN_ACCESS));
	rb_const_set(klass, rb_intern("ACCESS"), INT2NUM(IN_ACCESS));

	rb_const_set(klass, rb_intern("IN_MODIFY"), INT2NUM(IN_MODIFY));
	rb_const_set(klass, rb_intern("MODIFY"), INT2NUM(IN_MODIFY));

	rb_const_set(klass, rb_intern("IN_ATTRIB"), INT2NUM(IN_ATTRIB));
	rb_const_set(klass, rb_intern("ATTRIB"), INT2NUM(IN_ATTRIB));

	rb_const_set(klass, rb_intern("IN_CLOSE_WRITE"), INT2NUM(IN_CLOSE_WRITE));
	rb_const_set(klass, rb_intern("CLOSE_WRITE"), INT2NUM(IN_CLOSE_WRITE));

	rb_const_set(klass, rb_intern("IN_CLOSE_NOWRITE"), INT2NUM(IN_CLOSE_NOWRITE));
	rb_const_set(klass, rb_intern("CLOSE_NOWRITE"), INT2NUM(IN_CLOSE_NOWRITE));

	rb_const_set(klass, rb_intern("IN_OPEN"), INT2NUM(IN_OPEN));
	rb_const_set(klass, rb_intern("OPEN"), INT2NUM(IN_OPEN));

	rb_const_set(klass, rb_intern("IN_MOVED_FROM"), INT2NUM(IN_MOVED_FROM));
	rb_const_set(klass, rb_intern("MOVED_FROM"), INT2NUM(IN_MOVED_FROM));

	rb_const_set(klass, rb_intern("IN_MOVED_TO"), INT2NUM(IN_MOVED_TO));
	rb_const_set(klass, rb_intern("MOVED_TO"), INT2NUM(IN_MOVED_TO));

	rb_const_set(klass, rb_intern("IN_CREATE"), INT2NUM(IN_CREATE));
	rb_const_set(klass, rb_intern("CREATE"), INT2NUM(IN_CREATE));

	rb_const_set(klass, rb_intern("IN_DELETE"), INT2NUM(IN_DELETE));
	rb_const_set(klass, rb_intern("DELETE"), INT2NUM(IN_DELETE));

	rb_const_set(klass, rb_intern("IN_DELETE_SELF"), INT2NUM(IN_DELETE_SELF));
	rb_const_set(klass, rb_intern("DELETE_SELF"), INT2NUM(IN_DELETE_SELF));

	// sent by any watch
	rb_const_set(klass, rb_intern("IN_UNMOUNT"), INT2NUM(IN_UNMOUNT));
	rb_const_set(klass, rb_intern("UNMOUNT"), INT2NUM(IN_UNMOUNT));

	rb_const_set(klass, rb_intern("IN_Q_OVERFLOW"), INT2NUM(IN_Q_OVERFLOW));
	rb_const_set(klass, rb_intern("Q_OVERFLOW"), INT2NUM(IN_Q_OVERFLOW));

	rb_const_set(klass, rb_intern("IN_IGNORED"), INT2NUM(IN_IGNORED));
	rb_const_set(klass, rb_intern("IGNORED"), INT2NUM(IN_IGNORED));

	// helper events
	rb_const_set(klass, rb_intern("IN_CLOSE"), INT2NUM(IN_CLOSE));
	rb_const_set(klass, rb_intern("CLOSE"), INT2NUM(IN_CLOSE));

	rb_const_set(klass, rb_intern("IN_MOVE"), INT2NUM(IN_MOVE));
	rb_const_set(klass, rb_intern("MOVE"), INT2NUM(IN_MOVE));

	// special flags
	rb_const_set(klass, rb_intern("IN_ISDIR"), INT2NUM(IN_ISDIR));
	rb_const_set(klass, rb_intern("ISDIR"), INT2NUM(IN_ISDIR));

	rb_const_set(klass, rb_intern("IN_ONESHOT"), INT2NUM(IN_ONESHOT));	
	rb_const_set(klass, rb_intern("ONESHOT"), INT2NUM(IN_ONESHOT));	

	rb_const_set(klass, rb_intern("IN_ALL_EVENTS"), INT2NUM(IN_ALL_EVENTS));
	rb_const_set(klass, rb_intern("ALL_EVENTS"), INT2NUM(IN_ALL_EVENTS));
}


static void rinotify_declare_instance_vars(VALUE klass) {
	// Array: holds a listing of all watch descriptors and the filename or directory name
	// that they happen to be watching
	VALUE watch_descriptors = rb_hash_new();
	rb_iv_set(klass, "@watch_descriptors", watch_descriptors);		
}
