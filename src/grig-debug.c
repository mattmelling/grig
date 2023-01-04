/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
    Grig:  Gtk+ user interface for the Hamradio Control Libraries.

    Copyright (C)  2001-2007  Alexandru Csete.

    Authors: Alexandru Csete <oz9aec@gmail.com>

    Comments, questions and bugreports should be submitted via
    http://sourceforge.net/projects/groundstation/
    More details can be found at the project home page:

            http://groundstation.sourceforge.net/
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
  
    You should have received a copy of the GNU General Public License
    along with this program; if not, visit http://www.fsf.org/
 
 
 
 
*/
/** \file grig-debug.c
 *  \brief Manage debug messages.
 *
 * The functions in this file are used to log debug messages generated by
 * hamlib and grig itself. The debug messages are printed on stderr and
 * saved into a file, if the debug handler has been initialised with a file
 * name.
 */
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <time.h>
#include <sys/time.h>
#include <hamlib/rig.h>
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "grig-debug.h"




static enum rig_debug_level_e dbglvl = RIG_DEBUG_NONE;

static gchar      *logfname = NULL;
/*static GIOChannel *logfile  = NULL;*/


const gchar *SRC_TO_STR[] = {N_("NONE"), N_("HAMLIB"), N_("GRIG")};


static void manage_debug_message (debug_msg_src_t source,
				  enum rig_debug_level_e debug_level,
				  const gchar *message);



/** \brief Initialise debug handler
 *  \param filename File name of log file or NULL.
 *
 * This function initialises the debug handler so that it is ready to
 * manage debug messages coming from hamlib. If logfile is not NULL, the
 * debug messages will be saved to this file as well.
 */
void
grig_debug_init  (gchar *filename)
{

        if (filename != NULL) {
                /*** FIXME: open file for write/append ***/
        }

        /* set debug handler */
        rig_set_debug_callback (grig_debug_hamlib_cb, NULL);
        
        /* send debug message to indicate readiness of debug handler */
        grig_debug_local (RIG_DEBUG_VERBOSE,
			  _("%s: Debug handler initialised."),
			  __FUNCTION__);

}


/** \brief Close debug handler.
 *
 * This function cleans up the debug handler. Any further debug messages
 * will be handled by hamlib.
 */
void
grig_debug_close ()
{

        /* send a final debug message */
        grig_debug_local (RIG_DEBUG_VERBOSE,
			  _("%s: Shutting down debug handler."),
			  __FUNCTION__);

        /* remove debug handler */
        rig_set_debug_callback (NULL, NULL);

        /* close log file if open */
}



/** \brief Manage hamlib debug messages */
int
grig_debug_hamlib_cb    (enum rig_debug_level_e debug_level,
			 rig_ptr_t user_data,
			 const char *fmt,
			 va_list ap)
{

	gchar          *msg;       /* formatted debug message */
	gchar         **msgv;      /* debug message line by line */
	guint           numlines;  /* the number of lines in the message */
	guint           i;


	if (debug_level > dbglvl)
		return RIG_OK;


	/* create character string and split it in case
	   it is a multi-line message */
	msg = g_strdup_vprintf (fmt, ap);

	/* remove trailing \n */
	g_strchomp (msg);

	/* split the message in case it is a multiline message */
	msgv = g_strsplit_set (msg, "\n", 0);
	numlines = g_strv_length (msgv);

	g_free (msg);

	/* for each line in msgv, call the real debug handler
	   which will print the debug message and save it to
	   a logfile
	*/
	for (i = 0; i < numlines; i++) {
		manage_debug_message (MSG_SRC_HAMLIB, debug_level, msgv[i]);
	}


	g_strfreev (msgv);
	
	return RIG_OK;

}


/** \brief Manage GRIG debug messages. */
int
grig_debug_local    (enum rig_debug_level_e debug_level,
		     const char *fmt,
		     ...)
{

	gchar      *msg;       /* formatted debug message */
	gchar     **msgv;      /* debug message line by line */
	guint       numlines;  /* the number of lines in the message */
	guint       i;
	va_list     ap;


	if (debug_level > dbglvl)
		return RIG_OK;


	va_start (ap, fmt);

	/* create character string and split it in case
	   it is a multi-line message */
	msg = g_strdup_vprintf (fmt, ap);

	/* remove trailing \n */
	g_strchomp (msg);

	/* split the message in case it is a multiline message */
	msgv = g_strsplit_set (msg, "\n", 0);
	numlines = g_strv_length (msgv);

	g_free (msg);

	/* for each line in msgv, call the real debug handler
	   which will print the debug message and save it to
	   a logfile
	*/
	for (i = 0; i < numlines; i++) {
		manage_debug_message (MSG_SRC_GRIG, debug_level, msgv[i]);
	}

	va_end(ap);

	g_strfreev (msgv);
	
	return RIG_OK;

}



/** \brief Get the name of the current log file.
 *  \return The name of the current log file or NULL.
 *
 * The function returns the name of the currently use log file or NULL
 * if the debug messages are not saved to file. In case of non NULL return
 * value, the function returns a newly allocated string that should be freed
 * by the caller when no longer needed.
 */
gchar *
grig_debug_get_log_file ()
{
        if (logfname != NULL) {
                return g_strdup (logfname);
        }
        else {
                return NULL;
        }
}




static void
manage_debug_message (debug_msg_src_t source,
              enum rig_debug_level_e debug_level,
              const gchar *message)
{
    /* get the time */
    GDateTime *tval = g_date_time_new_now_local ();
    gchar *msg_time = g_date_time_format (tval, "%Y/%m/%d %H:%M:%S");

    g_fprintf (stderr,
           "%s%s%s%s%d%s%s\n",
           msg_time,
           GRIG_DEBUG_SEPARATOR,
           SRC_TO_STR[source],
           GRIG_DEBUG_SEPARATOR,
           debug_level,
           GRIG_DEBUG_SEPARATOR,
           message);

    g_free(msg_time);
}


void
grig_debug_set_level (enum rig_debug_level_e level)
{
	if ((level >= RIG_DEBUG_NONE) && (level <= RIG_DEBUG_TRACE)) {

		dbglvl = level;
		rig_set_debug (level);
	}
}

int
grig_debug_get_level ()
{
	return (int) (dbglvl);
}

