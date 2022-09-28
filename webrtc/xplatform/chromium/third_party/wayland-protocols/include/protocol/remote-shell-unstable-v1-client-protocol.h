/* Generated by wayland-scanner 1.13.0 */

#ifndef REMOTE_SHELL_UNSTABLE_V1_CLIENT_PROTOCOL_H
#define REMOTE_SHELL_UNSTABLE_V1_CLIENT_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @page page_remote_shell_unstable_v1 The remote_shell_unstable_v1 protocol
 * Create remote desktop-style surfaces
 *
 * @section page_desc_remote_shell_unstable_v1 Description
 *
 * remote_shell allows clients to turn a wl_surface into a "real window"
 * which can be stacked and activated by the user.
 *
 * Warning! The protocol described in this file is experimental and backward
 * incompatible changes may be made. Backward compatible changes may be added
 * together with the corresponding interface version bump. Backward
 * incompatible changes are done by bumping the version number in the protocol
 * and interface names and resetting the interface version. Once the protocol
 * is to be declared stable, the 'z' prefix and the version number in the
 * protocol and interface names are removed and the interface version number is
 * reset.
 *
 * @section page_ifaces_remote_shell_unstable_v1 Interfaces
 * - @subpage page_iface_zcr_remote_shell_v1 - remote_shell
 * - @subpage page_iface_zcr_remote_surface_v1 - A desktop window
 * - @subpage page_iface_zcr_notification_surface_v1 - A notification window
 * @section page_copyright_remote_shell_unstable_v1 Copyright
 * <pre>
 *
 * Copyright 2016 The Chromium Authors.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * </pre>
 */
struct wl_surface;
struct zcr_notification_surface_v1;
struct zcr_remote_shell_v1;
struct zcr_remote_surface_v1;

/**
 * @page page_iface_zcr_remote_shell_v1 zcr_remote_shell_v1
 * @section page_iface_zcr_remote_shell_v1_desc Description
 *
 * The global interface that allows clients to turn a wl_surface into a
 * "real window" which is remotely managed but can be stacked, activated
 * and made fullscreen by the user.
 * @section page_iface_zcr_remote_shell_v1_api API
 * See @ref iface_zcr_remote_shell_v1.
 */
/**
 * @defgroup iface_zcr_remote_shell_v1 The zcr_remote_shell_v1 interface
 *
 * The global interface that allows clients to turn a wl_surface into a
 * "real window" which is remotely managed but can be stacked, activated
 * and made fullscreen by the user.
 */
extern const struct wl_interface zcr_remote_shell_v1_interface;
/**
 * @page page_iface_zcr_remote_surface_v1 zcr_remote_surface_v1
 * @section page_iface_zcr_remote_surface_v1_desc Description
 *
 * An interface that may be implemented by a wl_surface, for
 * implementations that provide a desktop-style user interface
 * and allows for remotely managed windows.
 *
 * It provides requests to treat surfaces like windows, allowing to set
 * properties like app id and geometry.
 *
 * The client must call wl_surface.commit on the corresponding wl_surface
 * for the remote_surface state to take effect.
 *
 * For a surface to be mapped by the compositor the client must have
 * committed both an remote_surface state and a buffer.
 * @section page_iface_zcr_remote_surface_v1_api API
 * See @ref iface_zcr_remote_surface_v1.
 */
/**
 * @defgroup iface_zcr_remote_surface_v1 The zcr_remote_surface_v1 interface
 *
 * An interface that may be implemented by a wl_surface, for
 * implementations that provide a desktop-style user interface
 * and allows for remotely managed windows.
 *
 * It provides requests to treat surfaces like windows, allowing to set
 * properties like app id and geometry.
 *
 * The client must call wl_surface.commit on the corresponding wl_surface
 * for the remote_surface state to take effect.
 *
 * For a surface to be mapped by the compositor the client must have
 * committed both an remote_surface state and a buffer.
 */
extern const struct wl_interface zcr_remote_surface_v1_interface;
/**
 * @page page_iface_zcr_notification_surface_v1 zcr_notification_surface_v1
 * @section page_iface_zcr_notification_surface_v1_desc Description
 *
 * An interface that may be implemented by a wl_surface to host
 * notification contents.
 * @section page_iface_zcr_notification_surface_v1_api API
 * See @ref iface_zcr_notification_surface_v1.
 */
/**
 * @defgroup iface_zcr_notification_surface_v1 The zcr_notification_surface_v1 interface
 *
 * An interface that may be implemented by a wl_surface to host
 * notification contents.
 */
extern const struct wl_interface zcr_notification_surface_v1_interface;

#ifndef ZCR_REMOTE_SHELL_V1_CONTAINER_ENUM
#define ZCR_REMOTE_SHELL_V1_CONTAINER_ENUM
/**
 * @ingroup iface_zcr_remote_shell_v1
 * containers for remote surfaces
 *
 * Determine how a remote surface should be stacked relative to other
 * shell surfaces.
 */
enum zcr_remote_shell_v1_container {
	/**
	 * default container
	 */
	ZCR_REMOTE_SHELL_V1_CONTAINER_DEFAULT = 1,
	/**
	 * system modal container
	 */
	ZCR_REMOTE_SHELL_V1_CONTAINER_OVERLAY = 2,
};
#endif /* ZCR_REMOTE_SHELL_V1_CONTAINER_ENUM */

#ifndef ZCR_REMOTE_SHELL_V1_STATE_TYPE_ENUM
#define ZCR_REMOTE_SHELL_V1_STATE_TYPE_ENUM
/**
 * @ingroup iface_zcr_remote_shell_v1
 * state types for remote surfaces
 *
 * Defines common show states for shell surfaces.
 */
enum zcr_remote_shell_v1_state_type {
	/**
	 * normal window state
	 */
	ZCR_REMOTE_SHELL_V1_STATE_TYPE_NORMAL = 1,
	/**
	 * minimized window state
	 */
	ZCR_REMOTE_SHELL_V1_STATE_TYPE_MINIMIZED = 2,
	/**
	 * maximized window state
	 */
	ZCR_REMOTE_SHELL_V1_STATE_TYPE_MAXIMIZED = 3,
	/**
	 * fullscreen window state
	 */
	ZCR_REMOTE_SHELL_V1_STATE_TYPE_FULLSCREEN = 4,
	/**
	 * pinned window state
	 */
	ZCR_REMOTE_SHELL_V1_STATE_TYPE_PINNED = 5,
	/**
	 * trusted pinned window state
	 */
	ZCR_REMOTE_SHELL_V1_STATE_TYPE_TRUSTED_PINNED = 6,
	/**
	 * moving window state
	 */
	ZCR_REMOTE_SHELL_V1_STATE_TYPE_MOVING = 7,
};
#endif /* ZCR_REMOTE_SHELL_V1_STATE_TYPE_ENUM */

#ifndef ZCR_REMOTE_SHELL_V1_ERROR_ENUM
#define ZCR_REMOTE_SHELL_V1_ERROR_ENUM
enum zcr_remote_shell_v1_error {
	/**
	 * given wl_surface has another role
	 */
	ZCR_REMOTE_SHELL_V1_ERROR_ROLE = 0,
	/**
	 * invalid notification key
	 */
	ZCR_REMOTE_SHELL_V1_ERROR_INVALID_NOTIFICATION_KEY = 1,
};
#endif /* ZCR_REMOTE_SHELL_V1_ERROR_ENUM */

#ifndef ZCR_REMOTE_SHELL_V1_LAYOUT_MODE_ENUM
#define ZCR_REMOTE_SHELL_V1_LAYOUT_MODE_ENUM
/**
 * @ingroup iface_zcr_remote_shell_v1
 * the layout mode
 *
 * Determine how a client should layout surfaces.
 */
enum zcr_remote_shell_v1_layout_mode {
	/**
	 * multiple windows
	 */
	ZCR_REMOTE_SHELL_V1_LAYOUT_MODE_WINDOWED = 1,
	/**
	 * restricted mode for tablet
	 */
	ZCR_REMOTE_SHELL_V1_LAYOUT_MODE_TABLET = 2,
};
#endif /* ZCR_REMOTE_SHELL_V1_LAYOUT_MODE_ENUM */

/**
 * @ingroup iface_zcr_remote_shell_v1
 * @struct zcr_remote_shell_v1_listener
 */
struct zcr_remote_shell_v1_listener {
	/**
	 * activated surface changed
	 *
	 * Notifies client that the activated surface changed.
	 */
	void (*activated)(void *data,
			  struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
			  struct wl_surface *gained_active,
			  struct wl_surface *lost_active);
	/**
	 * suggests a re-configuration of remote shell
	 *
	 * [Deprecated] Suggests a re-configuration of remote shell.
	 */
	void (*configuration_changed)(void *data,
				      struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
				      int32_t width,
				      int32_t height,
				      int32_t transform,
				      wl_fixed_t scale_factor,
				      int32_t work_area_inset_left,
				      int32_t work_area_inset_top,
				      int32_t work_area_inset_right,
				      int32_t work_area_inset_bottom,
				      uint32_t layout_mode);
	/**
	 * area of remote shell
	 *
	 * Defines an area of the remote shell used for layout. Each
	 * series of "workspace" events must be terminated by a "configure"
	 * event.
	 * @param is_internal 1 if screen is built-in
	 * @since 5
	 */
	void (*workspace)(void *data,
			  struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
			  uint32_t id_hi,
			  uint32_t id_lo,
			  int32_t x,
			  int32_t y,
			  int32_t width,
			  int32_t height,
			  int32_t inset_left,
			  int32_t inset_top,
			  int32_t inset_right,
			  int32_t inset_bottom,
			  int32_t transform,
			  wl_fixed_t scale_factor,
			  uint32_t is_internal);
	/**
	 * suggests configuration of remote shell
	 *
	 * Suggests a new configuration of the remote shell. Preceded by
	 * a series of "workspace" events.
	 * @since 5
	 */
	void (*configure)(void *data,
			  struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
			  uint32_t layout_mode);
};

/**
 * @ingroup iface_zcr_remote_shell_v1
 */
static inline int
zcr_remote_shell_v1_add_listener(struct zcr_remote_shell_v1 *zcr_remote_shell_v1,
				 const struct zcr_remote_shell_v1_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) zcr_remote_shell_v1,
				     (void (**)(void)) listener, data);
}

#define ZCR_REMOTE_SHELL_V1_DESTROY 0
#define ZCR_REMOTE_SHELL_V1_GET_REMOTE_SURFACE 1
#define ZCR_REMOTE_SHELL_V1_GET_NOTIFICATION_SURFACE 2

/**
 * @ingroup iface_zcr_remote_shell_v1
 */
#define ZCR_REMOTE_SHELL_V1_ACTIVATED_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_shell_v1
 */
#define ZCR_REMOTE_SHELL_V1_CONFIGURATION_CHANGED_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_shell_v1
 */
#define ZCR_REMOTE_SHELL_V1_WORKSPACE_SINCE_VERSION 5
/**
 * @ingroup iface_zcr_remote_shell_v1
 */
#define ZCR_REMOTE_SHELL_V1_CONFIGURE_SINCE_VERSION 5

/**
 * @ingroup iface_zcr_remote_shell_v1
 */
#define ZCR_REMOTE_SHELL_V1_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_shell_v1
 */
#define ZCR_REMOTE_SHELL_V1_GET_REMOTE_SURFACE_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_shell_v1
 */
#define ZCR_REMOTE_SHELL_V1_GET_NOTIFICATION_SURFACE_SINCE_VERSION 1

/** @ingroup iface_zcr_remote_shell_v1 */
static inline void
zcr_remote_shell_v1_set_user_data(struct zcr_remote_shell_v1 *zcr_remote_shell_v1, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) zcr_remote_shell_v1, user_data);
}

/** @ingroup iface_zcr_remote_shell_v1 */
static inline void *
zcr_remote_shell_v1_get_user_data(struct zcr_remote_shell_v1 *zcr_remote_shell_v1)
{
	return wl_proxy_get_user_data((struct wl_proxy *) zcr_remote_shell_v1);
}

static inline uint32_t
zcr_remote_shell_v1_get_version(struct zcr_remote_shell_v1 *zcr_remote_shell_v1)
{
	return wl_proxy_get_version((struct wl_proxy *) zcr_remote_shell_v1);
}

/**
 * @ingroup iface_zcr_remote_shell_v1
 *
 * Destroy this remote_shell object.
 *
 * Destroying a bound remote_shell object while there are surfaces
 * still alive created by this remote_shell object instance is illegal
 * and will result in a protocol error.
 */
static inline void
zcr_remote_shell_v1_destroy(struct zcr_remote_shell_v1 *zcr_remote_shell_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_shell_v1,
			 ZCR_REMOTE_SHELL_V1_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) zcr_remote_shell_v1);
}

/**
 * @ingroup iface_zcr_remote_shell_v1
 *
 * This creates an remote_surface for the given surface and gives it the
 * remote_surface role. A wl_surface can only be given a remote_surface
 * role once. If get_remote_surface is called with a wl_surface that
 * already has an active remote_surface associated with it, or if it had
 * any other role, an error is raised.
 *
 * See the documentation of remote_surface for more details about what an
 * remote_surface is and how it is used.
 */
static inline struct zcr_remote_surface_v1 *
zcr_remote_shell_v1_get_remote_surface(struct zcr_remote_shell_v1 *zcr_remote_shell_v1, struct wl_surface *surface, uint32_t container)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) zcr_remote_shell_v1,
			 ZCR_REMOTE_SHELL_V1_GET_REMOTE_SURFACE, &zcr_remote_surface_v1_interface, NULL, surface, container);

	return (struct zcr_remote_surface_v1 *) id;
}

/**
 * @ingroup iface_zcr_remote_shell_v1
 *
 * Creates a notification_surface for the given surface, gives it the
 * notification_surface role and associated it with a notification id.
 */
static inline struct zcr_notification_surface_v1 *
zcr_remote_shell_v1_get_notification_surface(struct zcr_remote_shell_v1 *zcr_remote_shell_v1, struct wl_surface *surface, const char *notification_key)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) zcr_remote_shell_v1,
			 ZCR_REMOTE_SHELL_V1_GET_NOTIFICATION_SURFACE, &zcr_notification_surface_v1_interface, NULL, surface, notification_key);

	return (struct zcr_notification_surface_v1 *) id;
}

#ifndef ZCR_REMOTE_SURFACE_V1_SYSTEMUI_VISIBILITY_STATE_ENUM
#define ZCR_REMOTE_SURFACE_V1_SYSTEMUI_VISIBILITY_STATE_ENUM
/**
 * @ingroup iface_zcr_remote_surface_v1
 * systemui visibility behavior
 *
 * Determine the visibility behavior of the system UI.
 */
enum zcr_remote_surface_v1_systemui_visibility_state {
	/**
	 * system ui is visible
	 */
	ZCR_REMOTE_SURFACE_V1_SYSTEMUI_VISIBILITY_STATE_VISIBLE = 1,
	/**
	 * system ui autohides and is not sticky
	 */
	ZCR_REMOTE_SURFACE_V1_SYSTEMUI_VISIBILITY_STATE_AUTOHIDE_NON_STICKY = 2,
	/**
	 * system ui autohides and is sticky
	 */
	ZCR_REMOTE_SURFACE_V1_SYSTEMUI_VISIBILITY_STATE_AUTOHIDE_STICKY = 3,
};
#endif /* ZCR_REMOTE_SURFACE_V1_SYSTEMUI_VISIBILITY_STATE_ENUM */

#ifndef ZCR_REMOTE_SURFACE_V1_ORIENTATION_ENUM
#define ZCR_REMOTE_SURFACE_V1_ORIENTATION_ENUM
/**
 * @ingroup iface_zcr_remote_surface_v1
 * window orientation
 *
 * The orientation of the window.
 */
enum zcr_remote_surface_v1_orientation {
	/**
	 * portrait
	 */
	ZCR_REMOTE_SURFACE_V1_ORIENTATION_PORTRAIT = 1,
	/**
	 * landscape
	 */
	ZCR_REMOTE_SURFACE_V1_ORIENTATION_LANDSCAPE = 2,
};
#endif /* ZCR_REMOTE_SURFACE_V1_ORIENTATION_ENUM */

#ifndef ZCR_REMOTE_SURFACE_V1_WINDOW_TYPE_ENUM
#define ZCR_REMOTE_SURFACE_V1_WINDOW_TYPE_ENUM
/**
 * @ingroup iface_zcr_remote_surface_v1
 * window type
 *
 * The type of the window.
 */
enum zcr_remote_surface_v1_window_type {
  /**
   * normal app window
   */
  ZCR_REMOTE_SURFACE_V1_WINDOW_TYPE_NORMAL = 1,
  /**
   * window is treated as systemui
   */
  ZCR_REMOTE_SURFACE_V1_WINDOW_TYPE_SYSTEM_UI = 2,
};
#endif /* ZCR_REMOTE_SURFACE_V1_WINDOW_TYPE_ENUM */

/**
 * @ingroup iface_zcr_remote_surface_v1
 * @struct zcr_remote_surface_v1_listener
 */
struct zcr_remote_surface_v1_listener {
	/**
	 * surface wants to be closed
	 *
	 * The close event is sent by the compositor when the user wants
	 * the surface to be closed. This should be equivalent to the user
	 * clicking the close button in client-side decorations, if your
	 * application has any...
	 *
	 * This is only a request that the user intends to close your
	 * window. The client may choose to ignore this request, or show a
	 * dialog to ask the user to save their data...
	 */
	void (*close)(void *data,
		      struct zcr_remote_surface_v1 *zcr_remote_surface_v1);
	/**
	 * surface state type changed
	 *
	 * [Deprecated] The state_type_changed event is sent by the
	 * compositor when the surface state changed.
	 *
	 * This is an event to notify that the window state changed in
	 * compositor. The state change may be triggered by a client's
	 * request, or some user action directly handled by the compositor.
	 * The client may choose to ignore this event.
	 */
	void (*state_type_changed)(void *data,
				   struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
				   uint32_t state_type);
	/**
	 * suggest a surface change
	 *
	 * The configure event asks the client to change surface state.
	 *
	 * The client must apply the origin offset to window positions in
	 * set_window_geometry requests.
	 *
	 * The states listed in the event are state_type values, and might
	 * change due to a client request or an event directly handled by
	 * the compositor.
	 *
	 * Clients should arrange their surface for the new state, and then
	 * send an ack_configure request with the serial sent in this
	 * configure event at some point before committing the new surface.
	 *
	 * If the client receives multiple configure events before it can
	 * respond to one, it is free to discard all but the last event it
	 * received.
	 * @since 5
	 */
	void (*configure)(void *data,
			  struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
			  int32_t origin_offset_x,
			  int32_t origin_offset_y,
			  struct wl_array *states,
			  uint32_t serial);
};

/**
 * @ingroup iface_zcr_remote_surface_v1
 */
static inline int
zcr_remote_surface_v1_add_listener(struct zcr_remote_surface_v1 *zcr_remote_surface_v1,
				   const struct zcr_remote_surface_v1_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) zcr_remote_surface_v1,
				     (void (**)(void)) listener, data);
}

#define ZCR_REMOTE_SURFACE_V1_DESTROY 0
#define ZCR_REMOTE_SURFACE_V1_SET_APP_ID 1
#define ZCR_REMOTE_SURFACE_V1_SET_WINDOW_GEOMETRY 2
#define ZCR_REMOTE_SURFACE_V1_SET_SCALE 3
#define ZCR_REMOTE_SURFACE_V1_SET_RECTANGULAR_SHADOW 4
#define ZCR_REMOTE_SURFACE_V1_SET_RECTANGULAR_SHADOW_BACKGROUND_OPACITY 5
#define ZCR_REMOTE_SURFACE_V1_SET_TITLE 6
#define ZCR_REMOTE_SURFACE_V1_SET_TOP_INSET 7
#define ZCR_REMOTE_SURFACE_V1_ACTIVATE 8
#define ZCR_REMOTE_SURFACE_V1_MAXIMIZE 9
#define ZCR_REMOTE_SURFACE_V1_MINIMIZE 10
#define ZCR_REMOTE_SURFACE_V1_RESTORE 11
#define ZCR_REMOTE_SURFACE_V1_FULLSCREEN 12
#define ZCR_REMOTE_SURFACE_V1_UNFULLSCREEN 13
#define ZCR_REMOTE_SURFACE_V1_PIN 14
#define ZCR_REMOTE_SURFACE_V1_UNPIN 15
#define ZCR_REMOTE_SURFACE_V1_SET_SYSTEM_MODAL 16
#define ZCR_REMOTE_SURFACE_V1_UNSET_SYSTEM_MODAL 17
#define ZCR_REMOTE_SURFACE_V1_SET_RECTANGULAR_SURFACE_SHADOW 18
#define ZCR_REMOTE_SURFACE_V1_SET_SYSTEMUI_VISIBILITY 19
#define ZCR_REMOTE_SURFACE_V1_SET_ALWAYS_ON_TOP 20
#define ZCR_REMOTE_SURFACE_V1_UNSET_ALWAYS_ON_TOP 21
#define ZCR_REMOTE_SURFACE_V1_ACK_CONFIGURE 22
#define ZCR_REMOTE_SURFACE_V1_MOVE 23
#define ZCR_REMOTE_SURFACE_V1_SET_ORIENTATION 24
#define ZCR_REMOTE_SURFACE_V1_SET_WINDOW_TYPE 25

/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_CLOSE_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_STATE_TYPE_CHANGED_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_CONFIGURE_SINCE_VERSION 5

/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_APP_ID_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_WINDOW_GEOMETRY_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_SCALE_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_RECTANGULAR_SHADOW_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_RECTANGULAR_SHADOW_BACKGROUND_OPACITY_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_TITLE_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_TOP_INSET_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_ACTIVATE_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_MAXIMIZE_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_MINIMIZE_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_RESTORE_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_FULLSCREEN_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_UNFULLSCREEN_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_PIN_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_UNPIN_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_SYSTEM_MODAL_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_UNSET_SYSTEM_MODAL_SINCE_VERSION 1
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_RECTANGULAR_SURFACE_SHADOW_SINCE_VERSION 2
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_SYSTEMUI_VISIBILITY_SINCE_VERSION 3
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_ALWAYS_ON_TOP_SINCE_VERSION 4
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_UNSET_ALWAYS_ON_TOP_SINCE_VERSION 4
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_ACK_CONFIGURE_SINCE_VERSION 5
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_MOVE_SINCE_VERSION 5
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_ORIENTATION_SINCE_VERSION 6
/**
 * @ingroup iface_zcr_remote_surface_v1
 */
#define ZCR_REMOTE_SURFACE_V1_SET_WINDOW_TYPE_SINCE_VERSION 7

/** @ingroup iface_zcr_remote_surface_v1 */
static inline void
zcr_remote_surface_v1_set_user_data(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) zcr_remote_surface_v1, user_data);
}

/** @ingroup iface_zcr_remote_surface_v1 */
static inline void *
zcr_remote_surface_v1_get_user_data(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	return wl_proxy_get_user_data((struct wl_proxy *) zcr_remote_surface_v1);
}

static inline uint32_t
zcr_remote_surface_v1_get_version(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	return wl_proxy_get_version((struct wl_proxy *) zcr_remote_surface_v1);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Unmap and destroy the window. The window will be effectively
 * hidden from the user's point of view, and all state will be lost.
 */
static inline void
zcr_remote_surface_v1_destroy(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) zcr_remote_surface_v1);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Set an application identifier for the surface.
 */
static inline void
zcr_remote_surface_v1_set_app_id(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, const char *app_id)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_SET_APP_ID, app_id);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * The window geometry of a window is its "visible bounds" from the
 * user's perspective. Client-side decorations often have invisible
 * portions like drop-shadows which should be ignored for the
 * purposes of aligning, placing and constraining windows.
 *
 * The window geometry is double buffered, and will be applied at the
 * time wl_surface.commit of the corresponding wl_surface is called.
 *
 * Once the window geometry of the surface is set once, it is not
 * possible to unset it, and it will remain the same until
 * set_window_geometry is called again, even if a new subsurface or
 * buffer is attached.
 *
 * If never set, the value is the full bounds of the output. This
 * updates dynamically on every commit.
 *
 * The arguments are given in the output coordinate space.
 *
 * The width and height must be greater than zero.
 */
static inline void
zcr_remote_surface_v1_set_window_geometry(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_SET_WINDOW_GEOMETRY, x, y, width, height);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Set a scale factor that will be applied to surface and all descendants.
 */
static inline void
zcr_remote_surface_v1_set_scale(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, wl_fixed_t scale)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_SET_SCALE, scale);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * [Deprecated] Request that surface needs a rectangular shadow.
 *
 * This is only a request that the surface should have a rectangular
 * shadow. The compositor may choose to ignore this request.
 *
 * The arguments are given in the output coordinate space and specifies
 * the inner bounds of the shadow.
 *
 * The arguments are given in the output coordinate space.
 * Specifying zero width and height will disable the shadow.
 */
static inline void
zcr_remote_surface_v1_set_rectangular_shadow(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_SET_RECTANGULAR_SHADOW, x, y, width, height);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Suggests the window's background opacity when the shadow is requested.
 */
static inline void
zcr_remote_surface_v1_set_rectangular_shadow_background_opacity(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, wl_fixed_t opacity)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_SET_RECTANGULAR_SHADOW_BACKGROUND_OPACITY, opacity);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Set a short title for the surface.
 *
 * This string may be used to identify the surface in a task bar,
 * window list, or other user interface elements provided by the
 * compositor.
 *
 * The string must be encoded in UTF-8.
 */
static inline void
zcr_remote_surface_v1_set_title(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, const char *title)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_SET_TITLE, title);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Set distance from the top of the surface to the contents.
 *
 * This distance typically represents the size of the window caption.
 */
static inline void
zcr_remote_surface_v1_set_top_inset(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, int32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_SET_TOP_INSET, height);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Make the surface active and bring it to the front.
 */
static inline void
zcr_remote_surface_v1_activate(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, uint32_t serial)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_ACTIVATE, serial);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Request that surface is maximized. The window geometry will be updated
 * to whatever the compositor finds appropriate for a maximized window.
 *
 * This is only a request that the window should be maximized. The
 * compositor may choose to ignore this request. The client should
 * listen to set_maximized events to determine if the window was
 * maximized or not.
 */
static inline void
zcr_remote_surface_v1_maximize(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_MAXIMIZE);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Request that surface is minimized.
 *
 * This is only a request that the window should be minimized. The
 * compositor may choose to ignore this request. The client should
 * listen to set_minimized events to determine if the window was
 * minimized or not.
 */
static inline void
zcr_remote_surface_v1_minimize(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_MINIMIZE);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Request that surface is restored. This restores the window geometry
 * to what it was before the window was minimized, maximized or made
 * fullscreen.
 *
 * This is only a request that the window should be restored. The
 * compositor may choose to ignore this request. The client should
 * listen to unset_maximized, unset_minimize and unset_fullscreen
 * events to determine if the window was restored or not.
 */
static inline void
zcr_remote_surface_v1_restore(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_RESTORE);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Request that surface is made fullscreen.
 *
 * This is only a request that the window should be made fullscreen.
 * The compositor may choose to ignore this request. The client should
 * listen to set_fullscreen events to determine if the window was
 * made fullscreen or not.
 */
static inline void
zcr_remote_surface_v1_fullscreen(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_FULLSCREEN);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Request that surface is made unfullscreen.
 *
 * This is only a request that the window should be made unfullscreen.
 * The compositor may choose to ignore this request. The client should
 * listen to unset_fullscreen events to determine if the window was
 * made unfullscreen or not.
 */
static inline void
zcr_remote_surface_v1_unfullscreen(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_UNFULLSCREEN);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Request that surface is pinned.
 *
 * This is only a request that the window should be pinned.
 * The compositor may choose to ignore this request. The client should
 * listen to state_changed events to determine if the window was
 * pinned or not. If trusted flag is non-zero, the app can prevent users
 * from exiting the pinned mode.
 */
static inline void
zcr_remote_surface_v1_pin(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, int32_t trusted)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_PIN, trusted);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Request that surface is unpinned.
 *
 * This is only a request that the window should be unpinned.
 * The compositor may choose to ignore this request. The client should
 * listen to unset_pinned events to determine if the window was
 * unpinned or not.
 */
static inline void
zcr_remote_surface_v1_unpin(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_UNPIN);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Suggests a surface should become system modal.
 */
static inline void
zcr_remote_surface_v1_set_system_modal(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_SET_SYSTEM_MODAL);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Suggests a surface should become non system modal.
 */
static inline void
zcr_remote_surface_v1_unset_system_modal(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_UNSET_SYSTEM_MODAL);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Request that surface needs a rectangular shadow.
 *
 * This is only a request that the surface should have a rectangular
 * shadow. The compositor may choose to ignore this request.
 *
 * The arguments are given in the remote surface coordinate space and
 * specifies inner bounds of the shadow. Specifying zero width and height
 * will disable the shadow.
 */
static inline void
zcr_remote_surface_v1_set_rectangular_surface_shadow(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_SET_RECTANGULAR_SURFACE_SHADOW, x, y, width, height);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Requests how the surface will change the visibility of the system UI when it is made active.
 */
static inline void
zcr_remote_surface_v1_set_systemui_visibility(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, uint32_t visibility)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_SET_SYSTEMUI_VISIBILITY, visibility);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Request that surface is made to be always on top.
 *
 * This is only a request that the window should be always on top.
 * The compositor may choose to ignore this request.
 *
 */
static inline void
zcr_remote_surface_v1_set_always_on_top(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_SET_ALWAYS_ON_TOP);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Request that surface is made to be not always on top.
 *
 * This is only a request that the window should be not always on top.
 * The compositor may choose to ignore this request.
 */
static inline void
zcr_remote_surface_v1_unset_always_on_top(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_UNSET_ALWAYS_ON_TOP);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * When a configure event is received, if a client commits the
 * surface in response to the configure event, then the client
 * must make an ack_configure request sometime before the commit
 * request, passing along the serial of the configure event.
 *
 * For instance, the compositor might use this information during display
 * configuration to change its coordinate space for set_window_geometry
 * requests only when the client has switched to the new coordinate space.
 *
 * If the client receives multiple configure events before it
 * can respond to one, it only has to ack the last configure event.
 *
 * A client is not required to commit immediately after sending
 * an ack_configure request - it may even ack_configure several times
 * before its next surface commit.
 *
 * A client may send multiple ack_configure requests before committing, but
 * only the last request sent before a commit indicates which configure
 * event the client really is responding to.
 */
static inline void
zcr_remote_surface_v1_ack_configure(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, uint32_t serial)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_ACK_CONFIGURE, serial);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Start an interactive, user-driven move of the surface.
 *
 * The compositor responds to this request with a configure event that
 * transitions to the "moving" state. The client must only initiate motion
 * after acknowledging the state change. The compositor can assume that
 * subsequent set_window_geometry requests are position updates until the
 * next state transition is acknowledged.
 *
 * The compositor may ignore move requests depending on the state of the
 * surface, e.g. fullscreen or maximized.
 */
static inline void
zcr_remote_surface_v1_move(struct zcr_remote_surface_v1 *zcr_remote_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_MOVE);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Set an orientation for the surface.
 */
static inline void
zcr_remote_surface_v1_set_orientation(struct zcr_remote_surface_v1 *zcr_remote_surface_v1, int32_t orientation)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_remote_surface_v1,
			 ZCR_REMOTE_SURFACE_V1_SET_ORIENTATION, orientation);
}

/**
 * @ingroup iface_zcr_remote_surface_v1
 *
 * Set the type of window. This is only a hint to the compositor and the
 * compositor is free to ignore it.
 */
static inline void zcr_remote_surface_v1_set_window_type(
    struct zcr_remote_surface_v1* zcr_remote_surface_v1,
    uint32_t type) {
  wl_proxy_marshal((struct wl_proxy*)zcr_remote_surface_v1,
                   ZCR_REMOTE_SURFACE_V1_SET_WINDOW_TYPE, type);
}

#define ZCR_NOTIFICATION_SURFACE_V1_DESTROY 0


/**
 * @ingroup iface_zcr_notification_surface_v1
 */
#define ZCR_NOTIFICATION_SURFACE_V1_DESTROY_SINCE_VERSION 1

/** @ingroup iface_zcr_notification_surface_v1 */
static inline void
zcr_notification_surface_v1_set_user_data(struct zcr_notification_surface_v1 *zcr_notification_surface_v1, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) zcr_notification_surface_v1, user_data);
}

/** @ingroup iface_zcr_notification_surface_v1 */
static inline void *
zcr_notification_surface_v1_get_user_data(struct zcr_notification_surface_v1 *zcr_notification_surface_v1)
{
	return wl_proxy_get_user_data((struct wl_proxy *) zcr_notification_surface_v1);
}

static inline uint32_t
zcr_notification_surface_v1_get_version(struct zcr_notification_surface_v1 *zcr_notification_surface_v1)
{
	return wl_proxy_get_version((struct wl_proxy *) zcr_notification_surface_v1);
}

/**
 * @ingroup iface_zcr_notification_surface_v1
 *
 * Unmap and destroy the notification surface.
 */
static inline void
zcr_notification_surface_v1_destroy(struct zcr_notification_surface_v1 *zcr_notification_surface_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_notification_surface_v1,
			 ZCR_NOTIFICATION_SURFACE_V1_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) zcr_notification_surface_v1);
}

#ifdef  __cplusplus
}
#endif

#endif
