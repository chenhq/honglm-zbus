#include <Python.h>
#include "../zbox/include//prelude.h"
#include "../zbox/include/zbusapi.h"

static PyObject *g_zbus_exception;


static PyObject*
py_zlog_use_stdout(PyObject* self, PyObject* args) {
	Py_BEGIN_ALLOW_THREADS
	zlog_use_stdout();
	Py_END_ALLOW_THREADS
	Py_RETURN_NONE;
}
static PyObject*
py_zlog_use_file(PyObject* self, PyObject* args) {
	char* file = ".";
	if(!PyArg_ParseTuple(args,"|s",&file)){
		return NULL;
	}
	Py_BEGIN_ALLOW_THREADS
	zlog_use_file(file);
	Py_END_ALLOW_THREADS
	Py_RETURN_NONE;
}

static PyObject*
py_zctx_new(PyObject* self, PyObject* args, PyObject* keywords) {
	int io_threads = 1;
	static char* kwlist[] = {"io_threads", NULL };
	if(!PyArg_ParseTupleAndKeywords(args, keywords, "|i", kwlist, &io_threads)){
		return NULL;
	}
	void* zctx = NULL;
	Py_BEGIN_ALLOW_THREADS
	zctx = zctx_new(io_threads);
	Py_END_ALLOW_THREADS
	if(!zctx){
		PyErr_SetString(g_zbus_exception, "zeromq context init error");
		return NULL;
	}
	return PyCObject_FromVoidPtr(zctx, NULL);
}


static PyObject*
py_zctx_destroy(PyObject* self, PyObject* args) {
	PyObject* pyzctx = NULL;
	if(!PyArg_ParseTuple(args,"O",&pyzctx)){
		return NULL;
	}
	void* zctx = PyCObject_AsVoidPtr(pyzctx);
	if (!zctx) {
		PyErr_SetString(g_zbus_exception, "zeromq context destroy error");
		return NULL;
	}
	Py_BEGIN_ALLOW_THREADS
	zctx_destroy(&zctx);
	Py_END_ALLOW_THREADS
	Py_RETURN_NONE;
}

static PyObject*
py_zbusconn_new(PyObject* self, PyObject* args, PyObject* keywords) {
	PyObject* pyzctx = NULL;
	char* host = "127.0.0.1";
	int port = 15555; 
	int verbose = 0;
	char* id = "";

	static char* kwlist[] = {"ctx", "host", "port", "verbose", "id", NULL };
	if(!PyArg_ParseTupleAndKeywords(args, keywords, "|Osiis", kwlist, 
		&pyzctx, &host, &port, &verbose, &id)){
		return NULL;
	}
	void* zctx = PyCObject_AsVoidPtr(pyzctx);
	if (!zctx) {
		PyErr_SetString(g_zbus_exception, "zeromq context missing");
		return NULL;
	}
	zbusconn_cfg_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	strcpy(cfg.host, host);
	cfg.ctx = zctx;
	cfg.port = port;
	cfg.verbose = verbose;
	strcpy(cfg.id, id); 

	zbusconn_t* conn = NULL;

	Py_BEGIN_ALLOW_THREADS
	conn = zbusconn_new(&cfg);
	Py_END_ALLOW_THREADS 
	if ( !conn ){
		PyErr_SetString(g_zbus_exception, "zbus connection init failed");
		return NULL;
	}
	return PyCObject_FromVoidPtr(conn, NULL);
}


static PyObject*
py_zbusconn_destroy(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL;
	if(!PyArg_ParseTuple(args,"O",&pyconn)){
		return NULL;
	}
	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection destroy error");
		return NULL;
	}
	Py_BEGIN_ALLOW_THREADS
	zbusconn_destroy(&conn);
	Py_END_ALLOW_THREADS

	Py_RETURN_NONE;
}

static PyObject*
py_zbusconn_reconnect(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL;  

	if(!PyArg_ParseTuple(args,"O",&pyconn)){
		return NULL;
	}
	
	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	}
	int rc = -1;
	Py_BEGIN_ALLOW_THREADS
	rc = zbusconn_reconnect(conn);
	Py_END_ALLOW_THREADS
	return PyLong_FromLong(rc);
}

static PyObject*
py_zbusconn_route(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL;
	PyObject* pysock_id = NULL;
	PyObject* pymsg = NULL; 

	if(!PyArg_ParseTuple(args,"OOO",&pyconn, &pysock_id, &pymsg)){
		return NULL;
	}
	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	}
	if(!PyString_Check(pysock_id)){
		PyErr_SetString(PyExc_TypeError, "expected socket id(str)");
		return NULL;
	}
	if(!PySequence_Check(pymsg)){
		PyErr_SetString(PyExc_TypeError, "expected sequence");
		return NULL;
	}

	int frame_count = PySequence_Size(pymsg);
	zmsg_t* req = zmsg_new();
	for(int i=0; i<frame_count; i++){
		PyObject* pyframe = PySequence_GetItem(pymsg, i);
		if(!PyString_Check(pyframe)){
			PyErr_SetString(PyExc_TypeError, "expected string");
			zmsg_destroy(&req);
			Py_DECREF(pyframe);
			return NULL;
		}
		Py_ssize_t pyframe_size;
		char* pyframe_content;
		PyString_AsStringAndSize(pyframe, &pyframe_content, &pyframe_size);
		zmsg_push_back(req, zframe_new((const char*)pyframe_content, pyframe_size));
		Py_DECREF(pyframe);
	}
	int rc; 
	Py_ssize_t pyframe_size;
	char* pyframe_content;
	PyString_AsStringAndSize(pysock_id, &pyframe_content, &pyframe_size);
	zframe_t* sock_frame = zframe_new((const char*)pyframe_content, pyframe_size);

	Py_BEGIN_ALLOW_THREADS
	rc = zbusconn_route(conn, sock_frame, req); 
	Py_END_ALLOW_THREADS
	return PyLong_FromLong(rc);
}

static PyObject*
py_zbusconn_probe(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL; 
	int timeout = 2500;

	if(!PyArg_ParseTuple(args,"O|i",&pyconn, &timeout)){
		return NULL;
	}
	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	} 
	int rc = -1;

	Py_BEGIN_ALLOW_THREADS
	rc = zbusconn_probe(conn, timeout); 
	Py_END_ALLOW_THREADS
	return PyLong_FromLong(rc);
}

static PyObject*
py_zbusconn_send(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL; 
	PyObject* pymsg = NULL; 
	
	if(!PyArg_ParseTuple(args,"OO",&pyconn, &pymsg)){
		return NULL;
	}
	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	}

	if(!PySequence_Check(pymsg)){
		PyErr_SetString(PyExc_TypeError, "expected sequence");
		return NULL;
	}

	int frame_count = PySequence_Size(pymsg);
	zmsg_t* req = zmsg_new();
	for(int i=0; i<frame_count; i++){
		PyObject* pyframe = PySequence_GetItem(pymsg, i);
		if(!PyString_Check(pyframe)){
			PyErr_SetString(PyExc_TypeError, "expected string");
			zmsg_destroy(&req);
			Py_DECREF(pyframe);
			return NULL;
		}
		Py_ssize_t pyframe_size;
		char* pyframe_content;
		PyString_AsStringAndSize(pyframe, &pyframe_content, &pyframe_size);
		zmsg_push_back(req, zframe_new((const char*)pyframe_content, pyframe_size));
		Py_DECREF(pyframe);
	}
	int rc; 

	Py_BEGIN_ALLOW_THREADS
	rc = zbusconn_send(conn, req); 
	Py_END_ALLOW_THREADS
	return PyLong_FromLong(rc);
}


static PyObject*
py_zbusconn_recv(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL;
	int timeout = -1;
	PyObject* result = NULL; 
	if(!PyArg_ParseTuple(args,"Oi",&pyconn, &timeout)){
		return NULL;
	}

	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	}

	zmsg_t* res = NULL;
	Py_BEGIN_ALLOW_THREADS
	res = zbusconn_recv(conn, timeout); 
	Py_END_ALLOW_THREADS
	if(!res){
		PyErr_SetString(g_zbus_exception, "request timeout--1");
		return NULL;
	}
	int res_size = zmsg_frame_size(res);
	result = PyList_New(res_size);
	for(int i=0; i<res_size; i++){
		zframe_t* frame = zmsg_pop_front(res);
		PyObject* pyframe = PyString_FromStringAndSize((char*)zframe_data(frame), zframe_size(frame));
		PyList_SetItem(result, i, pyframe);
		zframe_destroy(&frame);
	}
	zmsg_destroy(&res);
	return result;
}


static PyObject*
py_zbuscli_send(PyObject* self, PyObject* args, PyObject* keywords) {
	PyObject* pyconn = NULL;
	char* service = NULL;
	char* token = "";
	char* msg_id = NULL;
	char* peer_id = NULL;
	int timeout = 2500; //ms
	PyObject* pymsg = NULL;  

	static char* kwlist[] = {"conn", "service", "msg_id", "peer_id", "msg", "token", "timeout", NULL };
	if(!PyArg_ParseTupleAndKeywords(args, keywords, "OsssO|si", kwlist, 
		&pyconn, &service, &msg_id, &peer_id, &pymsg, &token, &timeout)){
			return NULL;
	}

	if(!PySequence_Check(pymsg)){
		PyErr_SetString(PyExc_TypeError, "expected sequence");
		return NULL;
	}
	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	}

	int frame_count = PySequence_Size(pymsg);
	zmsg_t* req = zmsg_new();
	for(int i=0; i<frame_count; i++){
		PyObject* pyframe = PySequence_GetItem(pymsg, i);
		if(!PyString_Check(pyframe)){
			PyErr_SetString(PyExc_TypeError, "expected string");
			zmsg_destroy(&req);
			Py_DECREF(pyframe);
			return NULL;
		}
		Py_ssize_t pyframe_size;
		char* pyframe_content;
		PyString_AsStringAndSize(pyframe, &pyframe_content, &pyframe_size);
		zmsg_push_back(req, zframe_new((const char*)pyframe_content, pyframe_size));
		Py_DECREF(pyframe);
	}
	
	asyn_ctrl_t ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	strcpy(ctrl.service, service);
	strcpy(ctrl.token, token);
	strcpy(ctrl.msg_id, msg_id);
	strcpy(ctrl.peer_id, peer_id);
	ctrl.timeout = timeout;

	zmsg_t* res = NULL;
	Py_BEGIN_ALLOW_THREADS
	res = zbuscli_send(conn, &ctrl, req); 
	Py_END_ALLOW_THREADS
	if(!res){
		PyErr_SetString(g_zbus_exception, "request timeout--2");
		return NULL;
	}

	int res_size = zmsg_frame_size(res);
	PyObject* result = PyList_New(res_size);
	result = PyList_New(res_size);
	for(int i=0; i<res_size; i++){
		zframe_t* frame = zmsg_pop_front(res);
		PyObject* pyframe = PyString_FromStringAndSize((char*)zframe_data(frame), zframe_size(frame));
		PyList_SetItem(result, i, pyframe);
		zframe_destroy(&frame);
	}
	zmsg_destroy(&res);
	return result;
}
 

static PyObject*
py_zbuscli_recv(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL;
	int ping_interval = -1; 
	if(!PyArg_ParseTuple(args,"Oi",&pyconn, &ping_interval)){
		return NULL;
	}

	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	}

	zmsg_t* res = NULL;
	Py_BEGIN_ALLOW_THREADS
	res = zbuscli_recv(conn, ping_interval); 
	Py_END_ALLOW_THREADS
	if(!res){
		PyErr_SetString(g_zbus_exception, "request timeout--3");
		return NULL;
	}

	int res_size = zmsg_frame_size(res);
	PyObject* result = PyList_New(res_size);
	for(int i=0; i<res_size; i++){
		zframe_t* frame = zmsg_pop_front(res);
		PyObject* pyframe = PyString_FromStringAndSize((char*)zframe_data(frame), zframe_size(frame));
		PyList_SetItem(result, i, pyframe);
		zframe_destroy(&frame);
	}
	zmsg_destroy(&res);
	return result;
}

static PyObject*
py_zbuscli_request(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL; 
	char* service = NULL;
	char* token = NULL;
	PyObject* pymsg = NULL;  
	int timeout = -1;

	if(!PyArg_ParseTuple(args,"OssO|i",&pyconn, &service, &token, &pymsg, &timeout)){
		return NULL;
	}

	if(!PySequence_Check(pymsg)){
		PyErr_SetString(PyExc_TypeError, "expected sequence");
		return NULL;
	}
	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	}

	int frame_count = PySequence_Size(pymsg);
	zmsg_t* req = zmsg_new();
	for(int i=0; i<frame_count; i++){
		PyObject* pyframe = PySequence_GetItem(pymsg, i);
		if(!PyString_Check(pyframe)){
			PyErr_SetString(PyExc_TypeError, "expected string");
			zmsg_destroy(&req);
			Py_DECREF(pyframe);
			return NULL;
		}
		Py_ssize_t pyframe_size;
		char* pyframe_content;
		PyString_AsStringAndSize(pyframe, &pyframe_content, &pyframe_size);
		zmsg_push_back(req, zframe_new((const char*)pyframe_content, pyframe_size));
		Py_DECREF(pyframe);
	}

	zmsg_t* res;
	Py_BEGIN_ALLOW_THREADS
	res = zbuscli_request(conn, service, token, req, timeout); 
	Py_END_ALLOW_THREADS
	if(!res){
		PyErr_SetString(g_zbus_exception, "request timeout--4");
		return NULL;
	} 

	int res_size = zmsg_frame_size(res);
	PyObject* result = PyList_New(res_size);
	for(int i=0; i<res_size; i++){
		zframe_t* frame = zmsg_pop_front(res);
		PyObject* pyframe = PyString_FromStringAndSize((char*)zframe_data(frame), zframe_size(frame));
		PyList_SetItem(result, i, pyframe);
		zframe_destroy(&frame);
	}
	zmsg_destroy(&res);
	return result;
}

static PyObject*
py_zbusmon_request(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL; 
	char* token = NULL;
	PyObject* pymsg = NULL;  
	int timeout = -1;

	if(!PyArg_ParseTuple(args,"OsO|i",&pyconn, &token, &pymsg, &timeout)){
		return NULL;
	}

	if(!PySequence_Check(pymsg)){
		PyErr_SetString(PyExc_TypeError, "expected sequence");
		return NULL;
	}
	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	}

	int frame_count = PySequence_Size(pymsg);
	zmsg_t* req = zmsg_new();
	for(int i=0; i<frame_count; i++){
		PyObject* pyframe = PySequence_GetItem(pymsg, i);
		if(!PyString_Check(pyframe)){
			PyErr_SetString(PyExc_TypeError, "expected string");
			zmsg_destroy(&req);
			Py_DECREF(pyframe);
			return NULL;
		}
		Py_ssize_t pyframe_size;
		char* pyframe_content;
		PyString_AsStringAndSize(pyframe, &pyframe_content, &pyframe_size);
		zmsg_push_back(req, zframe_new((const char*)pyframe_content, pyframe_size));
		Py_DECREF(pyframe);
	}
	zmsg_t* res;
	Py_BEGIN_ALLOW_THREADS
	res = zbusmon_request(conn, token, req, timeout); 
	Py_END_ALLOW_THREADS
	if(!res){
		PyErr_SetString(g_zbus_exception, "request timeout");
		return NULL;
	}

	int res_size = zmsg_frame_size(res);
	PyObject* result = PyList_New(res_size);
	for(int i=0; i<res_size; i++){
		zframe_t* frame = zmsg_pop_front(res);
		PyObject* pyframe = PyString_FromStringAndSize((char*)zframe_data(frame), zframe_size(frame));
		PyList_SetItem(result, i, pyframe);
		zframe_destroy(&frame);
	}
	zmsg_destroy(&res);
	return result;
}
 

static PyObject*
py_zbuswrk_recv(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL;
	PyObject* pyworker = NULL;
	PyObject* result = NULL;  
	if(!PyArg_ParseTuple(args,"OO", &pyconn, &pyworker)){
		return NULL;
	}
	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	}

	zbuswrk_t* worker = (zbuswrk_t*)PyCObject_AsVoidPtr(pyworker);
	if (!worker) {
		PyErr_SetString(g_zbus_exception, "zbus worker control missing");
		return NULL;
	}

	
	zmsg_t* res = NULL; 
	Py_BEGIN_ALLOW_THREADS
	res = zbuswrk_recv(conn, worker); 
	Py_END_ALLOW_THREADS
	if(!res){
		Py_RETURN_NONE;
	}
	 
	zframe_t *sock_id, *msg_id;
	zbuswrk_get_address(worker, &sock_id, &msg_id);
	zmsg_push_front(res, msg_id);
	zmsg_push_front(res, sock_id);

	int res_size = zmsg_frame_size(res);
	result = PyList_New(res_size);
	for(int i=0; i<res_size; i++){
		zframe_t* frame = zmsg_pop_front(res);
		PyObject* pyframe = PyString_FromStringAndSize((char*)zframe_data(frame), zframe_size(frame));
		PyList_SetItem(result, i, pyframe);
		zframe_destroy(&frame);
	}
	zmsg_destroy(&res);
	return result;
}

static PyObject*
py_zbuswrk_send(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL;
	PyObject* pyworker = NULL;
	PyObject* pymsg = NULL;  
	if(!PyArg_ParseTuple(args,"OOO",&pyconn, &pyworker, &pymsg)){
		return NULL;
	}
	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	}

	zbuswrk_t* worker = (zbuswrk_t*)PyCObject_AsVoidPtr(pyworker);
	if (!worker) {
		PyErr_SetString(g_zbus_exception, "zbus worker control missing");
		return NULL;
	}

	if(!PySequence_Check(pymsg)){
		PyErr_SetString(PyExc_TypeError, "expected sequence");
		return NULL;
	}

	int frame_count = PySequence_Size(pymsg);
	zmsg_t* req = zmsg_new();
	for(int i=0; i<frame_count; i++){
		PyObject* pyframe = PySequence_GetItem(pymsg, i);
		if(!PyString_Check(pyframe)){
			PyErr_SetString(PyExc_TypeError, "expected string");
			zmsg_destroy(&req);
			Py_DECREF(pyframe);
			return NULL;
		}
		Py_ssize_t pyframe_size;
		char* pyframe_content;
		PyString_AsStringAndSize(pyframe, &pyframe_content, &pyframe_size);
		zmsg_push_back(req, zframe_new((const char*)pyframe_content, pyframe_size));
		Py_DECREF(pyframe);
	}
	int rc = -1; 
	zframe_t* sock_id = zmsg_pop_front(req);;
	zframe_t* msg_id = zmsg_pop_front(req);
	zbuswrk_set_address(worker, sock_id, msg_id);

	Py_BEGIN_ALLOW_THREADS
	rc = zbuswrk_send(conn, worker, req); 
	Py_END_ALLOW_THREADS

	return PyLong_FromLong(rc);
}


static PyObject*
py_zbuswrk_new(PyObject* self, PyObject* args, PyObject* keywords) {
	char* service = NULL;
	char* mode = MODE_LB; 
	char* reg_token = "";
	char* acc_token = ""; 

	static char* kwlist[] = {"service", "mode", "register_token", "access_token", NULL };
	if(!PyArg_ParseTupleAndKeywords(args, keywords, "s|sss", kwlist, &service, &mode, &reg_token, &acc_token)){
		return NULL;
	}
	if(strcmp(mode, MODE_LB)!=0 && strcmp(mode, MODE_BC) !=0 && strcmp(mode, MODE_PUBSUB) !=0){
		PyErr_SetString(PyExc_TypeError, "mode not support");
		return NULL;
	}
	zbuswrk_cfg_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	strcpy(cfg.service, service);
	strcpy(cfg.mode, mode);
	strcpy(cfg.reg_token, reg_token);
	strcpy(cfg.acc_token, acc_token);

	zbuswrk_t* worker = zbuswrk_new(&cfg); 

	return PyCObject_FromVoidPtr(worker, NULL);
}

static PyObject*
py_zbuswrk_destroy(PyObject* self, PyObject* args) {
	PyObject* pyworker = NULL;
	if(!PyArg_ParseTuple(args,"O",&pyworker)){
		return NULL;
	}
	zbuswrk_t* worker = (zbuswrk_t*)PyCObject_AsVoidPtr(pyworker);
	if (!worker) {
		PyErr_SetString(g_zbus_exception, "zeromq context destroy error");
		return NULL;
	}
	zbuswrk_destroy(&worker);

	Py_RETURN_NONE;
}


static PyObject*
py_zbuswrk_subscribe(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL;
	PyObject* pyworker = NULL; 
	char* topic = "";
	if(!PyArg_ParseTuple(args,"OOs", &pyconn, &pyworker, &topic)){
		return NULL;
	}
	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	}

	zbuswrk_t* worker = (zbuswrk_t*)PyCObject_AsVoidPtr(pyworker);
	if (!worker) {
		PyErr_SetString(g_zbus_exception, "zbus worker control missing");
		return NULL;
	} 

	int rc = 0;
	Py_BEGIN_ALLOW_THREADS
	rc = zbuswrk_subscribe(conn, worker, topic); 
	Py_END_ALLOW_THREADS
	return PyLong_FromLong(rc);
}

static PyObject*
py_zbuswrk_unsubscribe(PyObject* self, PyObject* args) {
	PyObject* pyconn = NULL;
	PyObject* pyworker = NULL; 
	char* topic = "";
	if(!PyArg_ParseTuple(args,"OOs", &pyconn, &pyworker, &topic)){
		return NULL;
	}
	zbusconn_t* conn = (zbusconn_t*)PyCObject_AsVoidPtr(pyconn);
	if (!conn) {
		PyErr_SetString(g_zbus_exception, "zbus connection missing");
		return NULL;
	}

	zbuswrk_t* worker = (zbuswrk_t*)PyCObject_AsVoidPtr(pyworker);
	if (!worker) {
		PyErr_SetString(g_zbus_exception, "zbus worker control missing");
		return NULL;
	} 

	int rc = 0;
	Py_BEGIN_ALLOW_THREADS
	rc = zbuswrk_unsubscribe(conn, worker, topic); 
	Py_END_ALLOW_THREADS
	return PyLong_FromLong(rc);
}


static PyMethodDef PyzbusMethods[] = {
	{ "zlog_use_stdout", py_zlog_use_stdout,METH_VARARGS, "enable stdout log" },
	{ "zlog_use_file", py_zlog_use_file,METH_VARARGS, "enable file log" },

	{ "zctx_new", (PyCFunction)py_zctx_new,METH_VARARGS|METH_KEYWORDS, "zeromq context initialisation" },
	{ "zctx_destroy", py_zctx_destroy, METH_VARARGS, "zeromq context destroy" },
	
	{ "zbusconn_new", (PyCFunction)py_zbusconn_new,METH_VARARGS|METH_KEYWORDS, "zbus connection initialisation" },
	{ "zbusconn_destroy", py_zbusconn_destroy, METH_VARARGS, "zbus connection destroy" },
	{ "zbusconn_reconnect", py_zbusconn_reconnect, METH_VARARGS, "zbus reconnect" },
	
	{ "zbusconn_send", py_zbusconn_send, METH_VARARGS, "zbus connection send" },
	{ "zbusconn_recv", py_zbusconn_recv, METH_VARARGS, "zbus connection recv" }, 
	{ "zbusconn_route", py_zbusconn_route, METH_VARARGS, "zbus connection route" },
	{ "zbusconn_probe", py_zbusconn_probe, METH_VARARGS, "zbus connection probe" },
	
	{ "zbuscli_send", (PyCFunction)py_zbuscli_send,METH_VARARGS|METH_KEYWORDS, "zbus client send" },
	{ "zbuscli_recv", py_zbuscli_recv, METH_VARARGS, "zbus client recv" },
	{ "zbuscli_request", py_zbuscli_request, METH_VARARGS, "zbus client request" },
	
	{ "zbuswrk_new", (PyCFunction)py_zbuswrk_new,METH_VARARGS|METH_KEYWORDS, "worker control initialisation" },
	{ "zbuswrk_destroy", py_zbuswrk_destroy, METH_VARARGS, "worker control destroy" },
	
	{ "zbuswrk_send", py_zbuswrk_send, METH_VARARGS, "zbus worker send" },
	{ "zbuswrk_recv", py_zbuswrk_recv, METH_VARARGS, "zbus worker recv" },
	{ "zbuswrk_subscribe", py_zbuswrk_subscribe, METH_VARARGS, "zbus worker subscribe" },
	{ "zbuswrk_unsubscribe", py_zbuswrk_unsubscribe, METH_VARARGS, "zbus worker unsubscribe" },

	{ "zbusmon_request", py_zbusmon_request, METH_VARARGS, "zbus monitor command request" },
	{ NULL, NULL, 0, NULL } /* Sentinel */
};

PyMODINIT_FUNC init_pyzbus(void) {
	PyEval_InitThreads();
	PyObject* zbus;
	zbus = Py_InitModule("_pyzbus", PyzbusMethods);
	if (zbus == NULL)
		return;

	g_zbus_exception = PyErr_NewException("zbus.ZBusException", NULL, NULL);

	Py_INCREF(g_zbus_exception);
	PyModule_AddObject(zbus, "ZBusException", g_zbus_exception);
}

