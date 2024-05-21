/*
   Copyright (C) 2024 EasyRPG Project <https://github.com/EasyRPG/>.

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
#include <cstdio>
#include "gui.h"
#include <wx/filedlg.h>
#include <wx/aboutdlg.h>
#include <wx/dcbuffer.h>
#include <wx/rawbmp.h>
#include "main.h"

wxIMPLEMENT_APP_NO_MAIN(Lmu2Png);

enum {
	Event_Quit = wxID_EXIT,
	Event_About = wxID_ABOUT,
	Event_Generate = 100,
	Event_Save,
	Event_BrowseMap,
	Event_ResetMap,
	Event_BrowseDB,
	Event_ResetDB,
	Event_BrowseCS,
	Event_ResetCS,
	Event_Last
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_BUTTON(Event_Quit, MyFrame::OnQuit)
	EVT_BUTTON(Event_About, MyFrame::OnAbout)
	EVT_BUTTON(Event_Generate, MyFrame::OnGenerate)
	EVT_BUTTON(Event_Save, MyFrame::OnSave)

	EVT_BUTTON(Event_BrowseMap, MyFrame::OnBrowse)
	EVT_BUTTON(Event_ResetMap, MyFrame::OnReset)
	EVT_BUTTON(Event_BrowseDB, MyFrame::OnBrowse)
	EVT_BUTTON(Event_ResetDB, MyFrame::OnReset)
	EVT_BUTTON(Event_BrowseCS, MyFrame::OnBrowse)
	EVT_BUTTON(Event_ResetCS, MyFrame::OnReset)
wxEND_EVENT_TABLE()

bool Lmu2Png::OnInit() {
	if (!wxApp::OnInit())
		return false;

	// create and show window
	MyFrame *frame = new MyFrame();
	frame->Show(true);

	return true;
}

MyFrame::MyFrame():
	wxFrame(nullptr, wxID_ANY, "EasyRPG lmu2png - Convert your RPG Maker 2k/3 maps to images",
		wxDefaultPosition, wxSize(1000, 600)) {

	SetMinSize(wxSize(800, 520));

	// panel to hold content (needed on windows to have correct background)
	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTAB_TRAVERSAL, "Background");

	// left pane
	wxSizer *sl = new wxBoxSizer(wxVERTICAL);

	auto create_picker = [this, panel, sl](const char *text, int eventId) {
		sl->Add(new wxStaticText(panel, wxID_ANY, wxString("&") + text + ":"),
			wxSizerFlags().Expand().Border());
		sl->AddSpacer(2);

		wxSizer *s = new wxBoxSizer(wxHORIZONTAL);
		wxButton *b = new wxButton(panel, eventId, "Browse");
		b->SetToolTip("Search for file");
		wxStaticText *st = new wxStaticText(panel, wxID_ANY, "placeholder",
			wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE|wxST_ELLIPSIZE_START);
		wxButton *c = new wxButton(panel, eventId + 1, "Clear");
		c->SetToolTip("Reset Path to Default");

		s->Add(b, wxSizerFlags());
		s->Add(st, wxSizerFlags(1).Expand().Border());
		s->Add(c, wxSizerFlags());

		sl->Add(s, wxSizerFlags().Expand().Border(wxALL, 8).DoubleHorzBorder());
		sl->AddSpacer(4);
		return st;
	};
	m_stMap = create_picker("Map", Event_BrowseMap);
	m_stDB = create_picker("Database", Event_BrowseDB);
	m_stCS = create_picker("ChipSet", Event_BrowseCS);

	// replace placeholders
	ResetPath(Event_ResetMap);
	ResetPath(Event_ResetDB);
	ResetPath(Event_ResetCS);

	sl->Add(new wxStaticText(panel, wxID_ANY, "&Encoding:"), wxSizerFlags().Expand().Border());
	m_choiceEnc = new wxChoice(panel, wxID_ANY);
	const char *encodings[] = {
		"Automatic (detect)",
		"932 - Japanese (Shift-JIS)",
		"936 - Simplified Chinese (GB2312)",
		"949 - Korean (Unified Hangul)",
		"950 - Traditional Chinese (Big5)",
		"1250 - Central European",
		"1251 - Cyrillic",
		"1252 - Western European (Latin 1)"
	};
	m_choiceEnc->Set(wxArrayString(8, encodings));
	m_choiceEnc->SetSelection(0);
	sl->Add(m_choiceEnc, wxSizerFlags().Expand().DoubleHorzBorder());

	wxStaticBoxSizer *box = new wxStaticBoxSizer(wxVERTICAL, panel, "Options");
	wxStaticBox *slBox = box->GetStaticBox();
	auto create_checkbox = [this, box, slBox](const char* text, bool value = true) {
		wxCheckBox *b = new wxCheckBox(slBox, wxID_ANY, text);
		box->Add(b, wxSizerFlags().Border(wxLEFT|wxRIGHT));
		box->AddSpacer(2);
		b->SetValue(value);
		b->Bind(wxEVT_CHECKBOX, &MyFrame::OnOptionChange, this);
		return b;
	};
	m_cbBG = create_checkbox("Show &Background");
	m_cbLT = create_checkbox("Show &Lower Tiles");
	m_cbUT = create_checkbox("Show &Upper Tiles");
	m_cbEV = create_checkbox("Show E&vents");
	box->AddSpacer(6);
	m_cbIC = create_checkbox("&Ignore conditions", false);
	m_cbIC->SetToolTip("Always draw the first page of the event instead of finding the first page "
		"with no conditions");
	m_cbSM = create_checkbox("Simulate &Movement", false);
	m_cbSM->SetToolTip("For event pages with certain animation types, draw the middle frame "
		"instead of the frame specified for the page");

	sl->Add(box, wxSizerFlags().Expand().DoubleBorder());

	sl->Add(new wxStaticText(panel, wxID_ANY, "Main actions:"), wxSizerFlags().Expand().Border());
	wxSizer *s = new wxBoxSizer(wxHORIZONTAL);
	wxButton* btnGenerate = new wxButton(panel, Event_Generate, "&Generate...");
	s->Add(btnGenerate, wxSizerFlags().Border(wxRIGHT));
	wxButton* btnSave = new wxButton(panel, Event_Save, "&Save PNG image...");
	s->Add(btnSave, wxSizerFlags());
	sl->Add(s, wxSizerFlags().Centre().DoubleBorder(wxLEFT|wxRIGHT));

	s = new wxBoxSizer(wxHORIZONTAL);
	wxButton* btnAbout = new wxButton(panel, Event_About, "&About");
	s->Add(btnAbout, wxSizerFlags().Border(wxRIGHT));
	wxButton* btnQuit = new wxButton(panel, Event_Quit, "&Quit");
	s->Add(btnQuit, wxSizerFlags());
	sl->Add(s, wxSizerFlags().Centre().DoubleBorder());

	// right pane
	wxSizer *sr = new wxBoxSizer(wxVERTICAL);
	sr->Add(new wxStaticText(panel, wxID_ANY, "Map preview:"));

	m_canvas = new MyCanvas(panel);
	sr->Add(m_canvas, wxSizerFlags(1).Expand());

	// configure split view, right pane can grow bigger
	wxSizer* const sizerTop = new wxBoxSizer(wxHORIZONTAL);
	sizerTop->Add(sl, wxSizerFlags(2).Border());
	sizerTop->Add(sr, wxSizerFlags(3).Expand().Border());
	panel->SetSizerAndFit(sizerTop);

	// statusbar
	CreateStatusBar();
	SetStatusDefault();
}

void MyFrame::OnBrowse(wxCommandEvent& event) {
	wxString title = "";
	wxString wildcard = "";
	wxStaticText *st;
	bool *state;

	wxBusyCursor wait;

	switch (event.GetId()) {
		case Event_BrowseMap:
			title = "Open Map";
			wildcard = "Map files (*.lmu)|*.lmu";
			st = m_stMap;
			state = &m_mapSelected;
			break;

		case Event_BrowseDB:
			title = "Open Database";
			wildcard = "Database files (*.ldb)|*.ldb";
			st = m_stDB;
			state = &m_dbSelected;
			break;

		case Event_BrowseCS:
			title = "Open ChipSet";
			wildcard = "Graphic files (*.bmp,*.png,*.xyz)|*.bmp;*.png;*.xyz";
			st = m_stCS;
			state = &m_csSelected;
			break;

		default:
			wxLogDebug("OnBrowse: Unknown event Id=%d", event.GetId());
			return;
	}

	wxFileDialog dlg(this, title, wxEmptyString, wxEmptyString,
		wildcard, wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_CANCEL) return;

	*state = true;
	st->SetLabel(dlg.GetPath());
	st->SetToolTip(dlg.GetPath());
}

void MyFrame::OnReset(wxCommandEvent& event) {
	ResetPath(event.GetId());
}

void MyFrame::ResetPath(int id) {
	switch (id) {
		case Event_ResetMap:
			m_stMap->SetLabel("Not selected");
			m_stMap->SetToolTip("");
			m_mapSelected = false;
			break;

		case Event_ResetDB:
			m_stDB->SetLabel("Default");
			m_stDB->SetToolTip("Used from same directory as map.");
			m_dbSelected = false;
			break;

		case Event_ResetCS:
			m_stCS->SetLabel("Default");
			m_stCS->SetToolTip("Is queried from database.");
			m_csSelected = false;
			break;

		default:
			wxLogDebug("ResetPath: Unknown event Id=%d", id);
			return;
	}
}

void MyFrame::OnOptionChange(wxCommandEvent& WXUNUSED(event)) {
	SetStatusText("Changed Layer visibility");
	CallAfter(&MyFrame::Update);
}

void MyFrame::OnGenerate(wxCommandEvent& WXUNUSED(event)) {
	SetStatusText("Generating Image");
	CallAfter(&MyFrame::Update);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
	wxAboutDialogInfo aboutInfo;
	aboutInfo.SetName("lmu2png");
	aboutInfo.SetVersion(PACKAGE_VERSION);
	aboutInfo.SetDescription("Convert your RPG Maker 2k/3 maps to images.");
	aboutInfo.SetCopyright("(C) 2016-2024 EasyRPG project");
	aboutInfo.SetWebSite(PACKAGE_URL);
	const char *authors[] = {
		"20kdc",
		"carstene1ns",
		"Zegeri",
		"",
		"LMU2BMP Author:",
		"Gustavo Valiente (chano)",
		"",
		"Includes EasyRPG project code from the following authors:",
		"Hector Barreiro (damizean)",
		"Francisco de la Pena (fdelapena)"
	};
	aboutInfo.SetDevelopers(wxArrayString(10, authors));
	wxAboutBox(aboutInfo);
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event)) {
	Close(true);
}

void MyFrame::OnSave(wxCommandEvent& WXUNUSED(event)) {
	wxBusyCursor wait;

	wxFileDialog dlg(this, "Save PNG file", wxEmptyString, "map.png",
		"PNG files (*.png)|*.png", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_CANCEL) return;

	if (m_canvas->Save(dlg.GetPath()))
		SetStatusText("Image saved.");
	else
		SetStatusText("Could not save image!");
}

void guiErrorCallback(const std::string& error, ErrorCallbackParam param) {
	MyFrame *instance = static_cast<MyFrame*>(param);
	instance->SetStatusText("Error: " + error);
}

void MyFrame::Update() {
	wxBusyCursor wait;
	ImgConfig conf = {};

	if(!m_mapSelected) {
		SetStatusDefault();
		return;
	}

	// automatic is default
	if (m_choiceEnc->GetSelection() > 0) {
		conf.encoding = m_choiceEnc->GetString(m_choiceEnc->GetSelection());
		conf.encoding = conf.encoding.substr(0, conf.encoding.find(" "));
	}

	conf.map = std::string(m_stMap->GetLabel());
	if(m_dbSelected)
		conf.database = std::string(m_stDB->GetLabel());
	if(m_csSelected)
		conf.chipset = std::string(m_stCS->GetLabel());

	conf.no_background = !m_cbBG->IsChecked();
	conf.no_lowertiles = !m_cbLT->IsChecked();
	conf.no_uppertiles = !m_cbUT->IsChecked();
	conf.no_events = !m_cbEV->IsChecked();
	conf.ignore_conditions = m_cbIC->IsChecked();
	conf.simulate_movement = m_cbSM->IsChecked();

	// generate image
	int w, h;
	unsigned char *pixels = makeImage(conf, w, h, guiErrorCallback, (void *)this);
	if(!pixels)
		return;

	m_canvas->Load(pixels, w, h);
	delete[] pixels;

	SetStatusText("Image generated!");
}

wxBEGIN_EVENT_TABLE(MyCanvas, wxScrolledWindow)
	EVT_PAINT(MyCanvas::OnPaint)
wxEND_EVENT_TABLE()

MyCanvas::MyCanvas(wxWindow *parent):
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER) {
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	//SetBackgroundColour(*wxLIGHT_GREY);
}

void MyCanvas::Load(unsigned char *pixels, int w, int h) {
	m_bmp.Create(w, h, 32);
	if(!m_bmp.Ok()) {
		return;
	}

	wxAlphaPixelData bmdata(m_bmp);
	if(!bmdata) {
		return;
	}

	// convert image
	unsigned char *src = pixels;
	wxAlphaPixelData::Iterator dst(bmdata);
	for(int y = 0; y<h; y++) {
		dst.MoveTo(bmdata, 0, y);
		for(int x = 0; x < w; x++) {
			// wxBitmap contains rgb values pre-multiplied with alpha
			unsigned char a = src[3];
			dst.Red() = src[0] * a / 255;
			dst.Green() = src[1] * a / 255;
			dst.Blue() = src[2] * a / 255;
			dst.Alpha() = a;
			dst++;
			src += 4;
		}
	}

	// show it
	Refresh();
}

bool MyCanvas::Save(wxString path) {
	if(!m_bmp.IsOk()) return false;

	m_bmp.SaveFile(path, wxBITMAP_TYPE_PNG);
	return true;
}

void MyCanvas::OnPaint(wxPaintEvent& WXUNUSED(event)) {
	wxBufferedPaintDC dc(this);
	PrepareDC(dc);
	dc.Clear();
	dc.SetUserScale(2, 2);

	const wxSize size = GetClientSize();

#if 0
	// draw transparency checkerboard
	dc.SetPen(*wxTRANSPARENT_PEN);
	const wxBrush *brushes[2] = { wxWHITE_BRUSH, wxGREY_BRUSH };
	for(int y = 0; y < size.y/8; y++) {
		for(int x = 0; x < size.x/8; x++) {
			dc.SetBrush(*brushes[(x + y) % 2]);
			dc.DrawRectangle(x * 8, y * 8, 8, 8);
		}
	}
#endif

	if(m_bmp.IsOk()) {
		dc.DrawBitmap(m_bmp,
			dc.DeviceToLogicalX((size.x - 2 * m_bmp.GetWidth()) / 2),
			dc.DeviceToLogicalY((size.y - 2 * m_bmp.GetHeight()) / 2),
			true // use transparency
		);

		// draw info at bottom
		dc.SetUserScale(1.2, 1.2);
		wxString s = wxString::Format("Preview %dx%d @2x", m_bmp.GetWidth(), m_bmp.GetHeight());
		dc.DrawText(s, 0, size.y / 1.2 - 20);
	}
}
