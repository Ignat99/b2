#ifndef HEADER_31034C08BC4F4DB29843E1CA29A5664C // -*- mode:c++ -*-
#define HEADER_31034C08BC4F4DB29843E1CA29A5664C

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::string OpenFileDialogWindows(const std::vector<OpenFileDialog::Filter> &filters,
                                  const std::string &default_path);

std::string SaveFileDialogWindows(const std::vector<OpenFileDialog::Filter> &filters,
                                  const std::string &default_path);

std::string SelectFolderDialogWindows(const std::string &default_path);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif
