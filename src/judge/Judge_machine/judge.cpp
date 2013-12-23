#include "judge.hpp"
#include "main.hpp"
#include <sys/time.h>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <deque>

using namespace std;

void remove_trailing_spaces(string& str)
{
	string::iterator erase_begin=str.end();
	while(erase_begin!=str.begin() && isspace(*(erase_begin-1))) --erase_begin;
	str.erase(erase_begin, str.end());
}

string f_time(int a)
{
	string w;
	while(a>0)
	{
		w=static_cast<char>(a%10+'0')+w;
		a/=10;
	}
	if(w.size()<3) w.insert(0,"000",3-w.size());
	w.insert(w.size()-2, ".");
return w;
}

string task::check_on_test(const string& test, const string& time_limit)
{
	string command="ulimit -v "+this->memory_limit+"; timeout "+time_limit+" chroot --userspec=1001 chroot/ ./exec.e < "+this->_name+"tests/"+test+".in > "+outf_name+" ", output="<td>";
#ifdef SHOW_LOGS
	cerr << command << endl;
#endif
	output+=test;
	output+="</td>\n";
	// runtime
	timeval ts, te;
	gettimeofday(&ts, NULL);
	int ret=system(command.c_str());
	gettimeofday(&te, NULL);
	double cl=(te.tv_sec+static_cast<double>(te.tv_usec)/1000000)-(ts.tv_sec+static_cast<double>(ts.tv_usec)/1000000);
	// end of runtime && time calculating
	cl*=100;
	cl=floor(cl)/100;
	double dtime_limit=strtod(time_limit.c_str(), NULL);
	if(cl>=dtime_limit) // Time limit
		output+="<td class=\"tl_re\">Time limit</td>\n<td>";
	else if(ret!=0) // Runtime error
	{
		output+="<td class=\"tl_re\">Runtime error</td>\n<td>";
		this->min_group_ratio=0;
	}
	else // checking answer
	{
		fstream out(outf_name.c_str(), ios_base::in), ans((this->_name+"tests/"+test+".out").c_str(), ios_base::in);
		if(!out.good() && !ans.good())
		{
			output+="<td style=\"background: #ff7b7b\">Evaluation failure</td>\n<td>";
			goto end_part;
		}
		deque<string> out_in, ans_in;
		string out_tmp, ans_tmp;
		while(out.good() && ans.good())
		{
			getline(out, out_tmp);
			getline(ans, ans_tmp);
			remove_trailing_spaces(out_tmp);
			remove_trailing_spaces(ans_tmp);
			out_in.push_back(out_tmp);
			ans_in.push_back(ans_tmp);
		}
		while(!out_in.empty() && out_in.back().empty()) out_in.pop_back();
		while(!ans_in.empty() && ans_in.back().empty()) ans_in.pop_back();
		int line=-1;
		while(++line<out_in.size() && line<ans_in.size())
			if(ans_in[line]!=out_in[line])
			{
				output+="<td class=\"wa\">Wrong answer</td>\n<td>";
				this->min_group_ratio=0;
				goto end_part;
				/*cout << test << ": Wrong! time - " << fixed;
				cout.precision(3);
				cout << cl << "s >> line: " << line+1 << endl;
				if(wrongs_info)
				{
					cout << "Get:\n'" << out_in[line] << "'\nExpected:\n'" << ans_in[line] << '\'' << endl;;
				}
				remove(this->outf_name.c_str());
				return 1;*/
			}
		if(ans_in.size()>out_in.size())
		{
			output+="<td class=\"wa\">Wrong answer</td>\n<td>";
			this->min_group_ratio=0;
			goto end_part;
			/*cout << test << ": Wrong! time - " << fixed;
			cout.precision(3);
			cout << cl << "s >> line: " << line+1 << endl;
			if(wrongs_info)
			{
				cout << "Get:\n'EOF'\nExpected:\n'" << ans_in[line] << '\'' << endl;
			}
			remove(this->outf_name.c_str());
			return 1;*/
		}
		output+="<td class=\"ok\">OK</td>\n<td>";
	} // end of checking answer
end_part:
	output+=f_time(static_cast<int>(cl*100));
	output+="/";
	output+=f_time(static_cast<int>(dtime_limit*100));
	output+="</td>\n";
	// calculating current_ratio
	cl=dtime_limit-cl;
	if(cl<0)
		cl=0;
	dtime_limit/=2;
	double current_ratio=cl/dtime_limit;
	if(current_ratio<this->min_group_ratio)
		this->min_group_ratio=current_ratio;
	remove(this->outf_name.c_str());
return output;
}

string task::judge()
{
	fstream config((this->_name+"conf.cfg").c_str(), ios::in);
#ifdef SHOW_LOGS
	cerr << "Openig file: " << this->_name << "conf.cfg" << endl;
#endif
	if(!config.good()) return "<pre>Judge Error</pre>";
#ifdef SHOW_LOGS
	cerr << "Success!" << endl;
#endif
	config >> this->memory_limit;
	string out="<table style=\"margin-top: 5px\" class=\"table results\">\n<thead>\n<tr>\n<th style=\"min-width: 70px\">Test</th>\n<th style=\"min-width: 180px\">Result</th>\n<th style=\"min-width: 90px\">Time</th>\n<th style=\"min-width: 60px\">Result</th>\n</tr>\n</thead>\n<tbody>\n";
	long long max_score=0, total_score=0, group_score;
	string test_name, time_limit, group_buffer;
	int other_tests=0;
	while(config.good())
	{
		config >> test_name >> time_limit;
		if(!other_tests)
		{
			this->min_group_ratio=1;
			config >> other_tests >> group_score;
			max_score+=group_score;
			out+="<tr>\n";
			out+=this->check_on_test(test_name, time_limit);
			out+="<td class=\"groupscore\""+string(other_tests>1 ? " rowspan=\""+myto_string(other_tests)+"\"":"")+">";
			group_buffer="";
		}
		else
		{
			group_buffer+="<tr>\n";
			group_buffer+=this->check_on_test(test_name, time_limit);
			group_buffer+="</tr>\n";
		}
		--other_tests;
		if(!other_tests)
		{
			total_score+=group_score*this->min_group_ratio;
			out+=myto_string(group_score*this->min_group_ratio)+"/"+myto_string(group_score)+"</td>\n</tr>\n";
			out+=group_buffer;
		}
	}
	out+="</tbody>\n</table>";
	/*this->_total_time=this->_max_time=0;
	this->_longest_test="";
	vector<string>().swap(this->_test_names);
	vector<string>().swap(this->WA);
	bool show_wrongs_info=argc!=1;
	if(argc==1) this->make_list_of_tests();
	else
	for(int i=1; i<argc; ++i)
		this->_test_names.push_back(argv[i]);
	for(vector<string>::iterator current_test=this->_test_names.begin(); current_test!=this->_test_names.end(); ++current_test)
	{
		if(this->check_on_test(*current_test, argv[0], show_wrongs_info))
			this->WA.push_back(*current_test);
	}
	if(!this->WA.empty())
	{
		cout << "Wrong tests: " << this->WA.front();
		for(vector<string>::iterator i=this->WA.begin()+1; i!=this->WA.end(); ++i)
			cout << ", " << *i;
		cout << endl;
	}
	cout << "Total time - " << fixed;
	cout.precision(3);
	cout << this->_total_time << "s\nMax time - " << fixed;
	cout.precision(3);
	cout << this->_max_time << "s : " << this->_longest_test << endl;*/
return "<pre>Score: "+myto_string(total_score)+"/"+myto_string(max_score)+"\nStatus: Judged</pre>\n"+out;
}