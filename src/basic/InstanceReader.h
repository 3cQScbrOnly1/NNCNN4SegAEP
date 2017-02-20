#ifndef _READER_
#define _READER_

#include "Reader.h"
#include "N3L.h"
#include <sstream>

using namespace std;

class InstanceReader : public Reader {
public:
	InstanceReader() {
	}
	~InstanceReader() {
	}

	Instance *getNext() {
		m_instance.clear();
		string strLine;
		if (!my_getline(m_inf, strLine))
			return NULL;
		if (strLine.empty())
			return NULL;


		vector<string> vecInfo;
		split_bychar(strLine, vecInfo, ' ');
		m_instance.m_label = vecInfo[0];

		const int max_size = vecInfo.size();
		int seg_end;
		for (int idx = 1; idx < max_size; idx++) {
			const string str_info = vecInfo[idx];
			if (str_info.find("[a]") != -1 || str_info.find("[e]") != -1 || str_info.find("[p]") != -1) {
				seg_end = idx;
				break;
			}
			else
				m_instance.m_segs.push_back(vecInfo[idx]);
		}

		for (int idx = seg_end; idx < max_size; idx++) {
			const string str_info = vecInfo[idx];
			if (str_info.find("[a]") != -1)
				m_instance.m_attributes.push_back(str_info);
			if (str_info.find("[e]") != -1)
				m_instance.m_evalutions.push_back(str_info);
			if (str_info.find("[p]") != -1)
				m_instance.m_polarity = str_info;
		}

		return &m_instance;
	}
};

#endif

