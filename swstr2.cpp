﻿/* #include "swstr.h"  std::wstring stows (std::string& s) {     return converter.from_bytes(s); } std::wstring stows (std::string&& s) {     return converter.from_bytes(s); }  std::string wstos (std::wstring& ws) {     return converter.to_bytes (ws); } std::string wstos (std::wstring&& ws) {     return converter.to_bytes (ws); }  static std::set<wchar_t> abc; static std::vector<std::vector<wchar_t> > t_abc; static std::wstring* ws{nullptr};  void cToL(wchar_t c, std::string* s) { 	auto &ru = t_abc[0], &en = t_abc[1]; 	if (c == ru[0] || c == en[0]) { s->push_back ('A'); return; } 	if (c == ru[1] || c == en[1]) { s->push_back ('a'); return; } 	if (c == ru[4] || c == en[2]) { s->push_back ('B'); return; } 	if (c == ru[10] || c == en[8]) { s->push_back ('E'); return; } 	if (c == ru[11] || c == en[9]) { s->push_back ('e'); return; } 	if (c == ru[22] || c == en[20]) { s->push_back ('K'); return; } 	if (c== ru[26] || c==en[24]) { s->push_back ('M'); return; } 	if (c == ru[28] || c == en[14]) { s->push_back ('H'); return; } }  void loadABC (const char path[]) { 	const unsigned buff_size = 200; 	char buff[buff_size]; 	std::ifstream fin (path); 	while (!fin.eof ()) { 		fin.getline (buff, buff_size); 		std::wstring&& wb = stows (buff); 		t_abc.push_back (std::vector<wchar_t> ()); 		auto& l = t_abc.back(); 		for (auto& it : wb) { if (it != L' ') { abc.insert (it); l.push_back (it); } } 	} }  bool isDigit (wchar_t ch) { 	return (ch == L'0')||(ch == L'1')||(ch == L'2')||(ch == L'3')||(ch == L'4')||(ch == L'5')||(ch == L'6')||(ch == L'7')||(ch == L'8')||(ch == L'9'); }  bool isLetter (wchar_t ch) { 	return abc.count (ch); } */ 