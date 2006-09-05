//
// workspace.hh for pekwm
// Copyright (C) 2002 Claes Nasten <pekdon@gmx.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "workspaces.hh"
#include "client.hh"
#include "frame.hh"

#include <X11/Xatom.h> // for XA_WINDOW

#include <algorithm>

#ifdef DEBUG
#include <iostream>
using std::cerr;
using std::endl;
#endif // DEBUG
using std::vector;
using std::list;

//! @fn    void activate(void)
//! @brief Activates the desktop by unhiding all clients on it
void
Workspaces::Workspace::activate(bool focus)
{
	if (!m_client_list.size())
		return;

	list<Client*>::iterator it = m_client_list.begin();
	for (; it != m_client_list.end(); ++it) {
		// We only care about the active client
		if (*it == (*it)->getFrame()->getActiveClient()) {
			if (!(*it)->isIconified()) {
				(*it)->getFrame()->unhide(false); // don't restack ontop windows
			}
		}
	}

	if (focus) {
		Client *client = NULL;
		// Try to focus the last focused client
		if (m_last_focused_client) {
			list<Client*>::iterator it =
			 find(m_client_list.begin(), m_client_list.end(), m_last_focused_client);
			if (it != m_client_list.end())
				client = *it;
			else
				client = m_client_list.back()->getFrame()->getActiveClient();
		} else
			client = m_client_list.back()->getFrame()->getActiveClient();

		// Activate the client
		if (!client->isHidden())
			client->giveInputFocus();
	}
}

//! @fn    void deactivate(void)
//! @brief Deactivates the desktop by hiding all clients on it
void
Workspaces::Workspace::deactivate(void)
{
	if (!m_client_list.size())
		return;

	list<Client*>::iterator it = m_client_list.begin();
	for (; it != m_client_list.end(); ++it) {
		// We only care about the active client
		if (*it == (*it)->getFrame()->getActiveClient()) {
			if (!(*it)->isSticky()) {
				(*it)->getFrame()->hide();
			}
		}
	}
}

//! @fn    void raiseClient(Client *c)
//! @brief Moves the client to a proper place in the stacking order
//! @param c Client on the workspace to raise
void
Workspaces::Workspace::raiseClient(Client *c)
{
	if (!c)
		return;

	list<Client*>::iterator it =
		find(m_client_list.begin(), m_client_list.end(), c);

	if (it != m_client_list.end()) {
		m_client_list.erase(it); // erase it's previous position

		bool raised = false;
		// now let's find a new home for the client based on it's layer
		for (it = m_client_list.begin(); it != m_client_list.end(); ++it) {
			if ((*it)->getLayer() > c->getLayer()) {
				m_client_list.insert(it, c);
				raised = true;

				if (c == c->getFrame()->getActiveClient()) {
					stackWinUnderWin((*it)->getFrame()->getFrameWindow(),
													 c->getFrame()->getFrameWindow());
				}
				break;
			}
		}

		// well, this layer were too high
		if (!raised) {
			m_client_list.push_back(c);
#ifdef HARBOUR
			harbour->stackWindowOverOrUnder(c->getFrame()->getFrameWindow(), true);
#else // !HARBOUR
			XRaiseWindow(dpy, c->getFrame()->getFrameWindow());
#endif // HARBOUR
		}
	}
}

//! @fn    void lowerClient(Client *c)
//! @brief Moves the client to a proper place in the stacking order
//! @param c Client on the workspace to lower
void
Workspaces::Workspace::lowerClient(Client *c)
{
	if (!c)
		return;

	list<Client*>::iterator it =
		find(m_client_list.begin(), m_client_list.end(), c);

	if (it != m_client_list.end()) {
		m_client_list.erase(it); // erase it's previous position

		bool lowered = false;
		// now let's find a new home for the client based on it's layer
		for (it = m_client_list.begin(); it != m_client_list.end(); ++it) {
			if (c->getLayer() <= (*it)->getLayer()) {
				m_client_list.insert(it, c);
				lowered = true;

				if (c == c->getFrame()->getActiveClient()) {
					stackWinUnderWin((*it)->getFrame()->getFrameWindow(),
													 c->getFrame()->getFrameWindow());
				}
				break;
			}
		}

		// well, this layer were too high
		if (!lowered) {
			m_client_list.push_back(c);
#ifdef HARBOUR
			harbour->stackWindowOverOrUnder(c->getFrame()->getFrameWindow(), false);
#else // !HARBOUR
			XRaiseWindow(dpy, c->getFrame()->getFrameWindow());
#endif // HARBOUR
		}
	}
}

//! @fn    void stackClientAbove(Client *c, Window win)
//! @brief Places the Client above the window win
//! @param c Client to place.
//! @param win Window to place Client above.
//! @param restack Restack the X windows, defaults to true.
void
Workspaces::Workspace::stackClientAbove(Client *c, Window win, bool restack)
{
	list<Client*>::iterator old_pos =
		find(m_client_list.begin(), m_client_list.end(), c);

	if (old_pos != m_client_list.end()) {
		list<Client*>::iterator it = m_client_list.begin();
		for (; it != m_client_list.end(); ++it) {
			if (win == (*it)->getWindow()) {
				m_client_list.erase(old_pos);
				m_client_list.insert(++it, c);

				// Before restacking make sure we are the active client
				// also that there are two different frames
				if (restack) {
					if (c == c->getFrame()->getActiveClient()) {
						stackWinUnderWin((*--it)->getFrame()->getFrameWindow(),
														 c->getFrame()->getFrameWindow());
					}
				}
				break;
			}
		}
	}
}

//! @fn    void stackClientBelow(Client *c, Window win)
//! @brief Places the Client below the window win
//! @param c Client to place.
//! @param win Window to place Client under
//! @param restack Restack the X windows, defaults to true
void
Workspaces::Workspace::stackClientBelow(Client *c, Window win, bool restack)
{
	list<Client*>::iterator old_pos =
		find(m_client_list.begin(), m_client_list.end(), c);

	if (old_pos != m_client_list.end()) {
		list<Client*>::iterator it = m_client_list.begin();
		for (; it != m_client_list.end(); ++it) {
			if (win == (*it)->getWindow()) {
				m_client_list.erase(old_pos);
				m_client_list.insert(it, c);

				// Before restacking make sure we are the active client
				// also that there are two different frames
				if (restack) {
					if (c == c->getFrame()->getActiveClient()) {
						stackWinUnderWin((*it)->getFrame()->getFrameWindow(),
														 c->getFrame()->getFrameWindow());
					}
				}
				break;
			}
		}
	}
}

//! @fn    void stackWinUnderWin(Window win_over, Window win_under)
//! @brief Helper function to stack a window below another
//! @param win_over Window to place win_under under
//! @param win_under Window to place under win_over
void
Workspaces::Workspace::stackWinUnderWin(Window win_over, Window win_under)
{
	if (win_over == win_under)
		return;

	Window windows[2] = { win_over, win_under };
	XRestackWindows(dpy, windows, 2);
}

//! @fn    Workspaces(unsigned int num)
//! @brief Constructor for Workspaces class
//! @param num Numbers of workspaces to begin whith

Workspaces::Workspaces(ScreenInfo *s, Config *c,
											 EwmhAtoms *ewmh, unsigned int num
#ifdef HARBOUR
											 , Harbour *h
#endif // !HARBOUR
											 ) :
scr(s), cfg(c),
ewmh_atoms(ewmh),
#ifdef HARBOUR
harbour(h),
#endif // HARBOUR
m_num_workspaces(num),
m_active_workspace(0)
{
	if (m_num_workspaces < 1)
		m_num_workspaces = 1;

	// Create new set of workspaces
	for (unsigned int i = 0; i < m_num_workspaces; ++i)
#ifdef HARBOUR
		m_workspace_list.push_back(new Workspace(scr->getDisplay(), harbour, i, ""));
#else // !HARBOUR
		m_workspace_list.push_back(new Workspace(scr->getDisplay(), i, ""));
#endif // HARBOUR
}

Workspaces::~Workspaces()
{
	vector<Workspace*>::iterator it = m_workspace_list.begin();
	for (; it != m_workspace_list.end(); ++it) {
		delete *it;
	}
	m_workspace_list.clear();
}

//! @fn    setNumWorkspaces(unsigned int num_workspaces)
//! @brief Sets number of workspaces
//!
//! @param num_workspaces New number of workspacs
//! @todo Actually do something
void
Workspaces::setNumWorkspaces(unsigned int num_workspaces)
{
	if (num_workspaces < 1)
		num_workspaces = 1;

	if (num_workspaces == m_num_workspaces)
		return; // no need to change number of workspaces to the current number

#ifdef DEBUG
	cerr << __FILE__ << "@" << __LINE__ << ": "
			 << "Changing num_workspaces from " << m_num_workspaces << " to "
			 << num_workspaces << endl;
#endif // DEBUG

	unsigned int workspaces = m_num_workspaces;
	m_num_workspaces = num_workspaces;
	if (m_active_workspace >= num_workspaces)
		m_active_workspace = num_workspaces - 1;

	// We have more workspaces than we want, lets remove the last ones
	if (workspaces > num_workspaces) {
		for (unsigned int i = workspaces - 1; i >= num_workspaces; --i) {
			list<Client*>::iterator it =
				m_workspace_list[i]->getClientList()->begin();
			// make sure the clients are on a valid workspace
			for (; it != m_workspace_list[i]->getClientList()->end(); ++it) {
				(*it)->setWorkspace(num_workspaces - 1);
			}

			delete m_workspace_list[i];
		}
		m_workspace_list.resize(num_workspaces, NULL);

	} else { // We need more workspaces, lets create some
		for (unsigned int i =  workspaces; i < num_workspaces; ++i) {
#ifdef HARBOUR
			m_workspace_list.push_back(new Workspace(scr->getDisplay(), harbour, i, ""));
#else // !HARBOUR
			m_workspace_list.push_back(new Workspace(scr->getDisplay(), i, ""));
#endif // HARBOUR
		}
	}

	// Tell the rest of the world how many workspaces we have.
	XChangeProperty(scr->getDisplay(), scr->getRoot(),
									ewmh_atoms->net_number_of_desktops,
									XA_CARDINAL, 32, PropModeReplace,
									(unsigned char *) &m_num_workspaces, 1);

#ifdef DEBUG
	if (m_num_workspaces != m_workspace_list.size()) {
		cerr << __FILE__ << "@" << __LINE__ << ": "
				 << "m_num_workspaces ( " << m_num_workspaces << " ) != "
				 << "m_workpspace_list.size() ( " << m_workspace_list.size()
				 << " ) " << endl;
	}
#endif // DEBUG
}

//! @fn    void activate(unsigned int workspace)
//! @brief Activates new workspace and deactivates old
//! @param workspace Workspace to activate
void
Workspaces::activate(unsigned int workspace, bool focus)
{
	// I _don't_ check if it's allready active as I re-active workspaces when
	// I shrink the number of workspaces.
	if (workspace >= m_num_workspaces)
		return;

	if (workspace != m_active_workspace)
		m_workspace_list[m_active_workspace]->deactivate();
	m_workspace_list[workspace]->activate(focus);

	m_active_workspace = workspace;
}

//! @fn    void getTopClient(void)
//! @brief Returns the Client with the highest stacking
Client*
Workspaces::getTopClient(unsigned int workspace)
{
	if (workspace >= m_num_workspaces)
		return NULL;

	if (m_workspace_list[workspace]->getClientList()->size()) {
		return m_workspace_list[workspace]->
			getClientList()->back()->getFrame()->getActiveClient();
	}

	return NULL;
}

//! @fn    void updateClientList(void)
//! @brief Updates the Ewmh Client list and Stacking list hint.
void
Workspaces::updateClientList(void)
{
	list<Window> win_list;

	// Find clients we are going to include in the list
	for (unsigned int i = 0; i < m_num_workspaces; ++i) {
		list<Client*> *c_list = m_workspace_list[i]->getClientList();
		list<Client*>::iterator it = c_list->begin();

		if (cfg->getReportOnlyFrames()) {
			for (; it != c_list->end(); ++it) {
				if (!(*it)->skipTaskbar() &&
						((*it)->getFrame()
						 ? ((*it) == (*it)->getFrame()->getActiveClient())
						 : true)) {
					win_list.push_back((*it)->getWindow());
				}
			}
		} else {
			for (; it != c_list->end(); ++it) {
				if (!(*it)->skipTaskbar()) {
					win_list.push_back((*it)->getWindow());
				}
			}
		}
	}

	if (!win_list.size())
		return;

	Window *windows = new Window[win_list.size()];
	copy(win_list.begin(), win_list.end(), windows);

	// Uppdate the properties
	XChangeProperty(scr->getDisplay(), scr->getRoot(),
									ewmh_atoms->net_client_list,
									XA_WINDOW, 32, PropModeReplace,
									(unsigned char*) windows, win_list.size());

	XChangeProperty(scr->getDisplay(), scr->getRoot(),
									ewmh_atoms->net_client_list_stacking,
									XA_WINDOW, 32, PropModeReplace,
									(unsigned char*) windows, win_list.size());

	delete [] windows;
}

//! @fn    void checkFrameSnap(int &x, int &y, Frame *frame, unsigned int workspace)
//! @brief Tries to snap the frame against a nearby frame.
void
Workspaces::checkFrameSnap(int &x, int &y,
													 Frame *frame, unsigned int workspace)
{
	// TO-DO: CLEAN THIS!
	if (workspace >= m_num_workspaces)
		return;

	unsigned int height = frame->getActiveClient()->isShaded()
		? (frame->getTitleHeight() + frame->borderTop() + frame->borderBottom())
		: frame->getHeight();

	int x1 = x;
	int x2 = x1 + frame->getWidth();
	int y1 = y;
	int y2 = y1 + height;

	list<Client*> *c_list = m_workspace_list[workspace]->getClientList();
	list<Client*>::reverse_iterator it = c_list->rbegin();

	Frame *f = NULL;
	int snap = cfg->getFrameSnap();

	for (; it != c_list->rend(); ++it) {
		if (((*it) != (*it)->getFrame()->getActiveClient()) ||
				(*it)->isHidden() || (*it)->isShaded() ||
				((*it)->getLayer() != WIN_LAYER_NORMAL)) {
			continue;
		}

		f = (*it)->getFrame();

		bool x_snapped = false;
		bool y_snapped = false;

		// check snap
		if (x2 >= (f->getX() - snap) && (x2 <= f->getX() + snap)) {
			if (isBetween(y1, y2, f->getY(), f->getY() + f->getHeight())) {
					x = f->getX() - frame->getWidth();
					if (y_snapped)
						break;
					x_snapped = true;
			}
		} else if ((x1 >= (signed)  (f->getX() + f->getWidth() - snap)) &&
							 (x1 <= (signed) (f->getX() + f->getWidth() + snap))) {
			if (isBetween(y1, y2, f->getY(), f->getY() + f->getHeight())) {
					x = f->getX() + f->getWidth();
					if (y_snapped)
						break;
					x_snapped = true;
			}
		}

		if (y2 >= (f->getY() - snap) && (y2 <= f->getY() + snap)) {
			if (isBetween(x1, x2, f->getX(), f->getX() + f->getWidth())) {
				y = f->getY() - height;
				if (x_snapped)
					break;
				y_snapped = true;
			}
		} else if ((y1 >= (signed) (f->getY() + f->getHeight() - snap)) &&
							 (y1 <= (signed) (f->getY() + f->getHeight() + snap))) {
			if (isBetween(x1, x2, f->getX(), f->getX() + f->getWidth())) {
				y = f->getY() + f->getHeight();
				if (x_snapped)
					break;
				y_snapped = true;
			}
		}
	}
}
