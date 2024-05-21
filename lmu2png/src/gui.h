/*
   Copyright (C) 2023 EasyRPG Project <https://github.com/EasyRPG/>.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */
#ifndef GUI_H
#define GUI_H

#include <memory>
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

class Lmu2Png : public wxApp {
public:
	Lmu2Png() {}
    virtual bool OnInit() override;
};

class MyCanvas : public wxScrolledWindow {
public:
	MyCanvas(wxWindow *parent);

	void Load(unsigned char *pixels, int w, int h);
	bool Save(wxString path);
	void OnPaint(wxPaintEvent &event);

private:
	wxBitmap m_bmp;

	wxDECLARE_EVENT_TABLE();
};

class MyFrame : public wxFrame {
public:
	MyFrame();

	void Update();

	void OnBrowse(wxCommandEvent& event);
	void OnReset(wxCommandEvent& event);
	void ResetPath(int id);
	void OnOptionChange(wxCommandEvent& event);
	void OnGenerate(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

	inline void SetStatusDefault() {
		SetStatusText("Select a map...");
	}

private:
	MyCanvas *m_canvas;

	// widgets we need to query
	wxCheckBox   *m_cbBG, *m_cbLT, *m_cbUT, *m_cbEV, *m_cbIC, *m_cbSM;
	wxStaticText *m_stMap, *m_stDB, *m_stCS;
	wxChoice     *m_choiceEnc;

	// app state
	bool m_mapSelected, m_dbSelected, m_csSelected;

	wxDECLARE_EVENT_TABLE();
};

wxDECLARE_APP(Lmu2Png);

#endif
