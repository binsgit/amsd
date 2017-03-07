/*
    This file is part of AMSD.
    Copyright (C) 2016-2017  CloudyReimu <cloudyreimu@gmail.com>

    AMSD is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AMSD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMSD.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../amsd.hpp"

const char *scripts_path = "/etc/ams/srtac_scripts.json";
static pthread_mutex_t scripts_fileio_lock = PTHREAD_MUTEX_INITIALIZER;
static shared_timed_mutex map_lock;
shared_timed_mutex scripts_memio_lock;

json_t *srtac_scripts = NULL;

atomic_size_t taskscount(0);
multimap<string, SuperRTACSession *> rtactasks;

static bool load_scripts(){
	if (srtac_scripts)
		return true;
	json_error_t err;

	pthread_mutex_lock(&scripts_fileio_lock);
	if (srtac_scripts)
		return true;
	srtac_scripts = json_load_file(scripts_path, 0, &err);
	pthread_mutex_unlock(&scripts_fileio_lock);

	return !(srtac_scripts == NULL);
}

static int save_scripts(){
	pthread_mutex_lock(&scripts_fileio_lock);
	int ret = json_dump_file(srtac_scripts, scripts_path, 0);
	pthread_mutex_unlock(&scripts_fileio_lock);

	return ret;
}

int amsd_operation_supertac(json_t *in_data, json_t *&out_data){

	if (!load_scripts())
		srtac_scripts = json_array();

	json_t *j_op = json_object_get(in_data, "op");
	json_t *j_script = NULL;
	json_t *j_scripts;
	json_t *j_script_name_reqd, *j_script_content_reqd;
	const char *script_name_reqd, *script_content = NULL;
	json_t *j_script_name, *j_script_content;
	json_t *j_exec_ips_reqd, *j_exec_ip_reqd, *j_exec_username_reqd = NULL, *j_exec_passwd_reqd = NULL;
	json_t *j_tasks, *j_task, *j_task_obj_buf;
	string exec_username_reqd, exec_passwd_reqd;
	const char *ipbuf = NULL;
	string s_ipbuf;
	SuperRTACSession *srtac = NULL;
	size_t i;
	int uflag = 0;
	string op;

	if (!j_op || !json_is_string(j_op))
		return -1;

	op = json_string_value(j_op);

	if (op == "get_scripts") {
		j_scripts = json_array();

		scripts_memio_lock.lock_shared();

		json_array_foreach(srtac_scripts, i, j_script) {
			if (json_is_object(j_script)) {
				json_array_append(j_scripts, j_script);
			}
		}

		uflag = json_object_set_new(out_data, "scripts", j_scripts);

		scripts_memio_lock.unlock_shared();
		return uflag;

	} else if (op == "get_script") {
		j_script_name_reqd = json_object_get(in_data, "name");
		if (!j_script_name_reqd || !json_is_string(j_script_name_reqd))
			return -1;
		script_name_reqd = json_string_value(j_script_name_reqd);

		scripts_memio_lock.lock_shared();
		json_array_foreach(srtac_scripts, i, j_script) {
			if (json_is_object(j_script)) {
				j_script_name = json_object_get(j_script, "name");
				if (j_script_name && json_is_string(j_script))
					if (0 == strcmp(json_string_value(j_script_name), script_name_reqd)) {
						json_object_set(out_data, "script", j_script);
						break;
					}
			}
		}
		scripts_memio_lock.unlock_shared();

		return 0;

	} else if (op == "add_script") {
		j_script_content_reqd = json_object_get(in_data, "content");
		if (!j_script_content_reqd || !json_is_string(j_script_content_reqd))
			return -1;

		j_script_name_reqd = json_object_get(in_data, "name");
		if (!j_script_name_reqd || !json_is_string(j_script_name_reqd))
			return -1;
		script_name_reqd = json_string_value(j_script_name_reqd);

		scripts_memio_lock.lock();
		json_array_foreach(srtac_scripts, i, j_script) {
			if (json_is_object(j_script)) {
				j_script_name = json_object_get(j_script, "name");
				if (j_script_name && json_is_string(j_script_name))
					if (0 == strcmp(json_string_value(j_script_name), script_name_reqd)) {
						uflag = 1;
						break;
					}
			}
		}

		if (uflag) {
			json_object_del(j_script, "content");
			json_object_del(j_script, "mtime");
		} else {
			j_script = json_object();
			json_object_set(j_script, "name", j_script_name_reqd);
		}

		json_object_set(j_script, "content", j_script_content_reqd);
		json_object_set_new(j_script, "mtime", json_integer(time(NULL)));

		if (!uflag) {
			json_array_append_new(srtac_scripts, j_script);
		}

		scripts_memio_lock.unlock();

		return save_scripts();

	} else if (op == "del_script") {
		j_script_name_reqd = json_object_get(in_data, "name");
		if (!j_script_name_reqd || !json_is_string(j_script_name_reqd))
			return -1;

		script_name_reqd = json_string_value(j_script_name_reqd);

		scripts_memio_lock.lock();
		json_array_foreach(srtac_scripts, i, j_script) {
			if (json_is_object(j_script)) {
				j_script_name = json_object_get(j_script, "name");
				if (j_script_name && json_is_string(j_script_name))
					if (0 == strcmp(json_string_value(j_script_name), script_name_reqd)) {
						uflag = json_array_remove(srtac_scripts, i);
						break;
					}
			}
		}

		scripts_memio_lock.unlock();
		return uflag;
	} else if (op == "tasks") {
		j_tasks = json_array();

		map_lock.lock_shared();
		for (auto thistask = rtactasks.begin(); thistask != rtactasks.end(); thistask++) {
			j_task = json_object();
			j_task_obj_buf = json_string(thistask->first.c_str());
			json_object_set_new(j_task, "ip", j_task_obj_buf);
			j_task_obj_buf = json_string(thistask->second->UUID.c_str());
			json_object_set_new(j_task, "uuid", j_task_obj_buf);
			j_task_obj_buf = json_string(thistask->second->ScriptName.c_str());
			json_object_set_new(j_task, "script", j_task_obj_buf);
			j_task_obj_buf = json_integer(thistask->second->StartTime);
			json_object_set_new(j_task, "start_time", j_task_obj_buf);
			j_task_obj_buf = json_string(thistask->second->Message().c_str());
			json_object_set_new(j_task, "msg", j_task_obj_buf);
			j_task_obj_buf = json_string(thistask->second->GetOutput().c_str());
			json_object_set_new(j_task, "output", j_task_obj_buf);
			j_task_obj_buf = json_string(thistask->second->GetLastOutputLine().c_str());
			json_object_set_new(j_task, "lastoutputline", j_task_obj_buf);

			json_array_append_new(j_tasks, j_task);
			taskscount++;
		}
		map_lock.unlock_shared();


		j_task_obj_buf = json_integer((json_int_t)taskscount);
		json_object_set_new(out_data, "count", j_task_obj_buf);
		json_object_set_new(out_data, "status", j_tasks);

		return 0;

	} else if (op == "exec_script") {
		j_script_name_reqd = json_object_get(in_data, "name");
		if (!j_script_name_reqd || !json_is_string(j_script_name_reqd))
			return -1;

		j_exec_ips_reqd = json_object_get(in_data, "ips");
		if (!j_exec_ips_reqd || !json_is_array(j_exec_ips_reqd))
			return -1;

		j_exec_username_reqd = json_object_get(in_data, "username");

		if (j_exec_username_reqd)
			if (!json_is_string(j_exec_username_reqd))
				return -1;
			else {
				j_exec_passwd_reqd = json_object_get(in_data, "passwd");
				if (!j_exec_passwd_reqd || !json_is_string(j_exec_passwd_reqd))
					return -1;
				else {
					exec_username_reqd = string(json_string_value(j_exec_username_reqd));
					exec_passwd_reqd = string(json_string_value(j_exec_passwd_reqd));
				}
			}

		script_name_reqd = json_string_value(j_script_name_reqd);

		scripts_memio_lock.lock_shared();
		json_array_foreach(srtac_scripts, i, j_script) {
			if (json_is_object(j_script)) {
				j_script_name = json_object_get(j_script, "name");
				if (j_script_name && json_is_string(j_script_name))
					if (0 == strcmp(json_string_value(j_script_name), script_name_reqd)) {
						j_script_content = json_object_get(j_script, "content");
						if (j_script_content && json_is_string(j_script_content)) {
							script_content = json_string_value(j_script_content);
							uflag = 1;
							break;
						} else
							return -2;
					}
			}
		}


		scripts_memio_lock.unlock_shared();

		if (!script_content)
			return -1;

		map_lock.lock();
		json_array_foreach(j_exec_ips_reqd, i, j_exec_ip_reqd) {
			if (json_is_string(j_exec_ip_reqd)) {
				ipbuf = json_string_value(j_exec_ip_reqd);
				s_ipbuf = ipbuf;

				srtac = new SuperRTACSession;

				srtac->IP = s_ipbuf;
				srtac->Script = string(script_content);
				srtac->ScriptName = string(json_string_value(j_script_name));

				if (j_exec_username_reqd) {
					srtac->UserName = exec_username_reqd;
					srtac->Password = exec_passwd_reqd;
				}

				srtac->Exec();
				rtactasks.insert(pair<string, SuperRTACSession *>(s_ipbuf, srtac));
			}
		}
		map_lock.unlock();

		return 0;

	} else if (op == "clear_tasks") {
		map_lock.lock();
		// See stackoverflow.com question 14511860
		for (auto thistask = rtactasks.begin(); thistask != rtactasks.end(); ) {
			if (thistask->second->Status() >= 0x80 ) {
				delete thistask->second;
				thistask = rtactasks.erase(thistask);
				taskscount--;
			} else
				++thistask;
		}

		map_lock.unlock();
		return 0;
	}

}


void SuperRTACSession::ExecInstance(SuperRTACSession *m) {

	m->RTACSession.IP = m->IP;
	m->RTACSession.UserName = (m->UserName == "" ? "root" : m->UserName);
	m->RTACSession.Password = m->Password;

	m->RTACSession.Command = "cat > /tmp/.ams.supertac." + m->UUID + ".sh << " + m->UUID +
				 "\n" + m->Script + "\n" +
				 m->UUID + "\nsh /tmp/.ams.supertac." + m->UUID + ".sh 2>&1";

	m->RTACSession.NonBlockingExec();

}

void *SuperRTACSession::ExecThread(void *p) {
	ExecInstance((SuperRTACSession *)p);
	pthread_exit(NULL);
}

void SuperRTACSession::Exec() {
	pthread_create(&ThreadId, &_pthread_detached, &ExecThread, this);
}

GeneralStatus SuperRTACSession::Status() {
	return RTACSession.Status;
}

string SuperRTACSession::Message() {
	if (RTACSession.Status == ConnectionFailure)
		return amsd_strerror(RTACSession.Status, RTACSession.ConnectionErrno);
	else
		return amsd_strerror(RTACSession.Status);
}

string SuperRTACSession::GetOutput() {
	vector<uint8_t> buf = RTACSession.GetReadBuffer();
	char *sbuf = (char *)&(buf[0]);

	return string(sbuf);
}

string SuperRTACSession::GetLastOutputLine() {
	string buf = GetOutput();
	char *sbuf = &(buf[0]);

	char *last2 = strrnchr(sbuf, '\n', buf.length(), 2);

	if (!last2)
		return buf;

	return string(last2);
}
