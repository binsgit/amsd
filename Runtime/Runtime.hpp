//
// Created by root on 17-4-1.
//

#ifndef AMSD_RUNTIME_HPP
#define AMSD_RUNTIME_HPP

#include "../amsd.hpp"

using namespace std;

extern Reimu::SQLAutomator db_controller, db_user, db_issue, db_summary, db_pool, db_device,
	db_module_policy, db_module_avalon7;


namespace AMSD {
    class Database : Reimu::SQLAutomator, Reimu::SQLAutomator::ColumnSpec {
    public:
	static int Init();
    };

    class RuntimeData {
    public:
	static int Init();

	static string Path_RuntimeDir;
	static string Path_ConfigDir;

	class TimeStamp {
	public:
	    static time_t CurrentDataCollection();
	    static void CurrentDataCollection(time_t t);

	    static time_t LastDataCollection();
	    static void LastDataCollection(time_t t);
	};
    };

    class Config {
    public:
	static shared_timed_mutex Lock;

	static void Init();
	static int Load(string filename);
	static int Save(string filename);

    };
}

#endif //AMSD_RUNTIME_HPP
