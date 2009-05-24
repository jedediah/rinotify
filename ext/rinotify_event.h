/*****************************************************
 * copyright (C) 2007 by Rob Merrell
 * rob@migrob.com
 *
 * ***************************************************/

#ifndef RINOTIFY_EVENT_H
#define RINOTIFY_EVENT_H

#include "ruby.h"
#include <sys/inotify.h>

// RInotifyEvent class
VALUE rb_cRInotifyEvent;

/* Ruby Prototypes */

// returns a new RInotifyEvent object.  This method should only be called by RInotify.each_event
VALUE rb_rinotify_event_new(struct inotify_event*);


/*
 * call-seq:
 *	RInotifyEvent.name -> event name
 *
 *	Returns the name of the event if the watched file is a directory.  If it is a file
 *	nil is returned.  In this case the event name can be found by looking the watch descriptor
 *	up using RInotify.watch_descriptors.
 *
 *	Example Usage:
 *	rinotify_event.name -> "/home/rob/desktop/tmp/test"
 */
VALUE rb_rinotify_event_name(VALUE);


/*
 * call-seq:
 *	RInotifyEvent.watch_descriptor -> event watch descriptor
 *
 *	Returns the watch descriptor that the event belongs to.
 *
 *	Example Usage:
 *	rinotify_event.watch_descriptor -> 1
 */
VALUE rb_rinotify_event_watch_descriptor(VALUE);


/*
 * call-seq:
 *	RInotifyEvent.check_mask(mask) -> boolean
 *
 *	Returns true or false if the current event contains the mask(s)
 *
 *	Example Usage:
 *	rinotify_event->check_mask(RInotify::MODIFY) -> true
 *	rinotify_event->check_mask(RInotify::DELETE_SELF) -> false
 */
VALUE rb_rinotify_event_check_mask(VALUE, VALUE);

#endif
