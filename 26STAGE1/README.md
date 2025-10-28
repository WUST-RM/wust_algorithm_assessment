# 26赛季第一阶段考核题目
## 所有内容要求在Ubuntu下完成

## TASK1
* 题目： 在你的代码中使用第3方库
* 考察目的：学会安装/使用第三方库 熟练使用CMake与命令行
* 要求：自行安装一个/多个第3方库（要求使用编译安装并给出安装过程的记录），写一个小小的C++demo使用安装的第3方库，使用CMake进行编译，并给出编译过程的记录
* 如果你喜欢线性代数，可以尝试使用Eigen   如果你喜欢机器视觉，可以尝试使用OpenCV   如果你喜欢3D点云与几何处理，可以尝试使用 Open3D 或 PCL  如果你喜欢深度学习推理，可以尝试使用 ONNX Runtime 或 TensorRT等
如果你喜欢GUI图形界面，可以尝试 Qt 或 ImGui  如果你喜欢 2D/3D 图形渲染，可以尝试 OpenGL

## TASK2

* 题目：小江是个爱搞怪的孩子，每次他玩到别人的ubuntu都会忍不住在别人电脑上搞点怪，可是他不会使用脚本，每次搞怪都会花比较多的时间，很容易被人家发现，所以小江想请你设计一个脚本，一键完成搞怪环境的部署（有root权限）
* 需求：在别人开启终端时骂他一句,只需要运行一个脚本就可以完成（可选：开机启动/一键修改骂人内容而不是多骂一句）
* ### 出题者很文明，都怪小江太坏了
* 如何骂人：
```
sudo apt install espeak # 安装espeak,无需重复安装，该部分运行一次即可


espeak "fuck you,ha ha ha dashabi" #  speak
echo "笨蛋被我耍了吧"
```
* 考察内容： 熟练使用脚本及linux命令
* 给出脚本文件


## TASK3
* 题目：小牛喜欢使用yaml（使用方法在培训视频演示过）读取参数文件，但是普通的写法只能一次性的读取参数，他希望你设计一个管理器，能够在只调用一个函数的情况下，让最开始读取的参数被动态加载，而且程序不会崩溃，又想让这个管理器哪里都能用，不用重复创建
* 需求：实现一个yaml管理器，能够动态加载参数（只能调用一个函数实现），并且不会崩溃，可以很多独立的类中调用
* 考察内容： 建立OOP GP思想，熟练使用STL STD库
* 注意：本题目的代码可作为你后续开发的常用小库，尽量提升规范性适用性简洁性
* 展示效果：要求在代码中持续输出多个变量同时调用动态加载函数，并在后台修改参数文件，使得变量发生修改，给出运行视频


## TASK4
* 题目：小施正在开发一个数学计算库，他希望实现求各种函数的导数，但是他发现各种函数求导的公式各有不同（而且他自己不怎么会高等数学），代码量过于庞大，他希望你能够帮助他设计一个自动微分的类
* 考察内容： 掌握 “前向自动微分（forward-mode automatic differentiation）“ 的原理，熟练使用 C++ 运算符重载、类封装与模板 理解 Ceres Solver 自动微分（Jet）机制 的实现思路
* 要求：请你实现一个 Dual 类，用于自动计算函数的导数，该类应能够同时表示一个数值和它对输入变量的导数，实现自动求导的功能。给出实现代码，适配的CMakeLists（可与其他题目共用）和测试用例及截图。

### Dual 类定义
```
class Dual {
public:
    double val;  // 数值部分 f(x)
    double der;  // 导数部分 f'(x)

    Dual(double value = 0.0, double derivative = 0.0);
};


//实现基本运算符重载

//使得 Dual 类支持以下操作

Dual operator+(const Dual&) const;
Dual operator-(const Dual&) const;
Dual operator*(const Dual&) const;
Dual operator/(const Dual&) const;


//实现常见数学函数

//支持以下函数，并能正确传播导数，每种至少原则3个（除了幂函数）可使用cmath

// === 基本初等函数 ===
friend Dual sin(const Dual&);
friend Dual cos(const Dual&);
friend Dual tan(const Dual&);
friend Dual cot(const Dual&);
friend Dual sec(const Dual&);
friend Dual csc(const Dual&);

// === 反三角函数 ===
friend Dual asin(const Dual&);
friend Dual acos(const Dual&);
friend Dual atan(const Dual&);
friend Dual acot(const Dual&);

// === 双曲函数 ===
friend Dual sinh(const Dual&);
friend Dual cosh(const Dual&);
friend Dual tanh(const Dual&);
friend Dual coth(const Dual&);

// === 指数与对数函数 ===
friend Dual exp(const Dual&);
friend Dual log(const Dual&);
friend Dual log10(const Dual&);

// === 幂函数 ===
friend Dual pow(const Dual&, double);
friend Dual pow(const Dual&, const Dual&);

// === 常见复合算术函数 ===
friend Dual sqrt(const Dual&);
friend Dual abs(const Dual&);
friend Dual floor(const Dual&);
friend Dual ceil(const Dual&);
friend Dual erf(const Dual&);
friend Dual sigmoid(const Dual&);   // 自定义常用函数 σ(x) = 1/(1+e^-x)

```

当用户定义如下代码时：
```
Dual x(3.0, 1.0);
Dual y = sin(x) + x * x;
```

程序应输出：
```
f(x) = sin(3) + 9
f'(x) = cos(3) + 2x
```
### 示例
示例 1：
```
Dual x(2.0, 1.0);
Dual y = sin(x) + x * x;
std::cout << "f(x) = " << y.val << ", f'(x) = " << y.der << std::endl;
```

输出：
```
f(x) = 4.9093, f'(x) = 3.5839
```
示例 2：
```
Dual x(1.0, 1.0);
Dual y = exp(x) / (1.0 + exp(x)); // Sigmoid
std::cout << "f(x) = " << y.val << ", f'(x) = " << y.der << std::endl;
```

输出：
```
f(x) = 0.731059, f'(x) = 0.196612
```
## TASK5
* 题目：小缪喜欢对类添加getxx setxx方法做到对成员变量的访问，但是他不希望访问到未定义的成员变量，请你设计一种“方法“（不一定是函数），对于已经定义的变量可以正常访问，对于未定义的则会抛出异常，并且有清除变量定义的方法就是除了getxx setxx（已经定义的变量可以正常访问，对于未定义的则会抛出异常） 还要有isSetxx（返回是否被定义） clearxx（清除定义状态，不需要清除内存）方法
* 需求：设计一种能够控制成员变量是否被定义，做到优雅的的getxx setxx isSetxx clearxx方法“（不一定是函数），可以看下面的进阶进一步思考
 ```
class Person {
private:
    string name;
    int age;
public:
    void setName(string name);
    string getName();
    bool isSetName();
    void clearName();
    void setAge(int age);
    int getAge();
    bool isSetAge();
    
}
```
* 进阶：小缪发现，对于一个类，每多一个成员变量，要多写很多函数，增加代码量降低可读性，他希望你能够做到简化，一键实现这么多的函数
* 提示：可以使用模板和宏

## TASK6
* 题目： 小涵在RoboMaster自瞄算法（相机输出图像，图像经过神经网络推理，将推理完的结果输入给预测器，预测器只能顺序接收结果---不能让比较晚的推理结果先进入预测器）设计了一个多线程推理系统，可以让很多图像被同时推理，但是由于推理时间不一，先进入推理的图像有可能被后处理完，导致比较晚的图像的推理结果先被送进预测器，如果她使用if(armor.timestamp < last_timestamp)return; 的话会导致比较多的推理结果直接被丢失，她希望你能帮帮她，让推理结果能够顺序进入预测器
* 提示：输入推理的图像时间戳已知。先从相机输出的图像更早/或者说先进入推理的图像更早。
* 要求：修改TASK6/task6.cpp,实现顺序track（表现为日志输出track/s接近cameraCallback/s，Error/s尽量最小）RoboMaster自瞄算法基本虚拟框架出题者已经实现
* 可以修改common.hpp内的结构体，cameraCallback和inferCallback内部可修改同时可以拓展更多方法和线程，track和printStatus不可修改


## TASK7
* 题目：小符同学和小装甲同学一起开发了一个自动瞄准系统，但是两个人平时很少交流，他开发他的她开发她的，一个人有自己的一套程序，但是两个人都瞧不起对面的垃圾代码，两个人使用的编程语言也不一样，但是他们的程序只要能够进行数据互通他们的两套程序就能被链接为一个完整的自瞄系统，现在他们需要你的帮助，实现数据互通。
* 需求：编写两个独立的程序（可均为C++，也可一个其他语言一个C++），实现进程间的通信，最少实现两种通信方案
```
//示例接口结构体
struct XiaoFUXiaoZhuangJIaInterface{
    std::string data;
    int id;
    std::chrono::steady_clock::time_point timestamp;//可转为int64_t
};
```
* 要求： 录制视频展示通信结果和对应方案的代码
* 可选：如果你尝试过opencv，可以尝试传输图像做到双端的实时图像显示