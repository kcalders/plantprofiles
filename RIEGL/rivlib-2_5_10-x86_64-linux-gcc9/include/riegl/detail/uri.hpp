// $Id$

#ifndef URI_HPP
#define URI_HPP

#include <string>

//*****************************************************************************
// uri parse to components
struct uri
{
    typedef std::string string;
    typedef std::string::size_type size_type;

    uri(
        const string& id
    )
    {
        const size_type npos = std::string::npos;
        size_type pos = 0;
        size_type fpos;
        fpos = id.find_first_of(":/?#", pos);
        if (npos != fpos && id[fpos] == ':') {
            scheme = id.substr(0,fpos);
            pos = fpos+1;
        }
        if (pos >= id.length()) return;

        fpos = id.find("//", pos);
        if (npos != fpos) {
            pos = fpos+2;
            if (pos >= id.length()) return;
            fpos = id.find_first_of("/?#", pos);
            if (npos == fpos) fpos = id.length();
            authority = id.substr(pos, fpos-pos);
            pos = fpos;
        }
        if (pos >= id.length()) return;

        fpos = id.find_first_of("?#", pos);
        if (npos == fpos) fpos = id.length();
        path = id.substr(pos, fpos-pos);
        pos = fpos;
        if (pos >= id.length()) return;

        fpos = id.find("?", pos);
        if (npos != fpos) {
            pos = fpos+1;
            if (pos >= id.length()) return;
            fpos = id.find_first_of("#", pos);
            if (npos == fpos) fpos = id.length();
            query = id.substr(pos, fpos-pos);
            pos = fpos;
        }
        if (pos >= id.length()) return;
        
        fpos = id.find("#", pos);
        if (npos != fpos) {
            pos = fpos+1;
            if (pos >= id.length()) return;
            fragment = id.substr(pos, npos);
        }
    }
    
    string scheme;
    string authority;
    string path;
    string query;
    string fragment;
};

#endif //URI_HPP
