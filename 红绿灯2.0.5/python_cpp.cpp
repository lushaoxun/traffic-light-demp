#include "util.h"

using namespace std;

PyObject * pClass;

void initPythonModule(){
	//初始化python
	Py_Initialize();

	//引入当前路径,否则下面模块不能正常导入
	PyRun_SimpleString("import sys");  
	PyRun_SimpleString("sys.path.append('./')");  

	//引入模块
	PyObject *pModule = PyImport_ImportModule("hld");
	//获取模块字典属性
	PyObject *pDict = PyModule_GetDict(pModule);

	//通过字典属性获取模块中的类
	pClass = PyDict_GetItemString(pDict, "ProcessImg");
}

void destroyPythonMoudle(){
	//释放python
	Py_Finalize();
}

void getImgCircles(char * imgPath, vector<vector<int>> &circles, int h4 = 1, 
	int h5 = 60, int h6 = 50, int h7 = 10, int h8 = 0, int h9 = 30){
		//实例化获取的类
		PyObject *pInstance = PyInstance_New(pClass, NULL, NULL);
		//调用类的方法

		PyObject *result = PyObject_CallMethod(pInstance, "process", "(s, i, i, i, i, i, i)", imgPath, h4, h5, h6, h7, h8, h9);
		int count;
		//将python类型的返回值转换为c/c++类型
		PyArg_Parse(result, "i", &count);

		vector<int> temp;
		for(int i = 0; i < count; i++){
			result = PyObject_CallMethod(pInstance, "getOne", "(s)", "i");
			int c;
			//将python类型的返回值转换为c/c++类型
			PyArg_Parse(result, "i", &c);

			temp.push_back(c);

		}
		for(int i = 0; i < temp.size() / 3; i++){
			vector<int> circle;
			circle.push_back(temp[3 * i]);
			circle.push_back(temp[3 * i + 1]);
			circle.push_back(temp[3 * i + 2]);

			circles.push_back(circle);
		}
}



int main78(int argc, char* argv[])
{
	initPythonModule();
	vector<vector<int>> circles;
	getImgCircles("3.png", circles);

	for(int i = 0; i < circles.size(); i++){
		cout << circles[i][0] << ", "
			<< circles[i][1] << ", "
			<< circles[i][2] << endl;
	}
	circles.clear();

	getImgCircles("4.png", circles);
	for(int i = 0; i < circles.size(); i++){
		cout << circles[i][0] << ", "
			<< circles[i][1] << ", "
			<< circles[i][2] << endl;
	}
	circles.clear();

	destroyPythonMoudle();
	getchar();
	return 0;
}