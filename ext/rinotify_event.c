#include "ruby.h"
#include "rinotify_event.h"

#include <sys/inotify.h>

VALUE rb_rinotify_event_new(struct inotify_event *event) {
	VALUE rinotify_event;

	// initialize the object
	rinotify_event = Data_Wrap_Struct(rb_cRInotifyEvent, NULL, free, event);
	rb_obj_call_init(rinotify_event, 0, NULL);

	return rinotify_event;
}


VALUE rb_rinotify_event_name(VALUE self) {
	VALUE name;
	struct inotify_event *event;	
	Data_Get_Struct(self, struct inotify_event, event);

	// if watching for events in a directory inotify will tell us the name
	if (event->len) {
		name = rb_str_new2(event->name);	
	} else {
	// watching a single file doesn't waste space with a filename so return nil
		name = Qnil;
	}

	return name;
}


VALUE rb_rinotify_event_watch_descriptor(VALUE self) {
	struct inotify_event *event;	
	Data_Get_Struct(self, struct inotify_event, event);

	return INT2NUM(event->wd);
}


VALUE rb_rinotify_event_check_mask(VALUE self, VALUE masks) {
	VALUE return_val;
	struct inotify_event *event;	
	Data_Get_Struct(self, struct inotify_event, event);

	// check if the mask is part of the event
	if (event->mask & NUM2INT(masks))
		return_val = Qtrue;
	else 
		return_val = Qfalse;

	return return_val;
}

