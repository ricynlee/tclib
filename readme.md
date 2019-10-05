# tclib
&copy; 2019 ricynlee
Released under MIT license

**A tool for mass testcase generation**

## 1 Numerous testcases in a single file
Mass and automated tests can be used to cover features of a product. It is preferable if all testcases can be described using one single file, so the *yamlite* is introduced here.
For example, a single-chip computer's boot features are to be tested. The testcases can be described as follows.
```yamlite
flash:
  - nand
  - nor
wire:
  - 1
  - 2
  - 4
  - 8
speed:
  - 30MHz
  - 60MHz
encrypted:
  - yes
  - no
encryption:
  - aes128
  - aes256
```
To make it easier to describe the following details, some terms are introduced here.

Term | Example | Note
-|-|-
key | `flash`, `encrypted` | Consists of letters (case-sensitive), digits, underscores
value | `4`, `30MHz`, `aes128` | Always regarded as a string/text
feature | `speed=30MHz`, `encrypted=yes`
combination | `flash=nand,wire=1,encrypted=yes`
testcase | A full *combination* like `flash=nand,wire=1,speed=30MHz,encrypted=yes,encryption=aes128`
condition | `0`(logical 0), `flash==nand`, `wire!=4`, `flash==nor \|\| !(encrypted==yes && encryption==aes128)` | Before `==` (or `!=`) must come a *key*, and a **valid** *value* comes after

**tclib** parses the yamlite input, picks a *value* of each *key*, concatenates those *features* to form a *testcase*.
**tclib** enumerates all possible *combinations*. For the yamlite above, there are 64 *testcases* in total.

### 1.1 Dependencies
From the example in the previous section it can be inferred that the *combination* `encrypted=no,encryption=aes128`. That is not supposed to exist, because if `encrypted=no`, there should not be the `encryption` key at all. In this case, a *dependency* can be added to a key. 
```yamlite
encryption:encrypted==yes
  - aes128
  - aes256
```
With the example above, `encryption` will be bypassed if `encrypted==no`.
> A *dependency* is a *condition*. It consists of operators like `&&`, as well as operands like `flash==nand`. Note that `==` and `!=` are not operators.

### 1.2 Constraints
There might be a situation, where `speed==60MHz` is only supported when `wire==1`. So a *constraint* can be added to the *value* of the *key* `speed`, like the example below.
```yamlite
speed:
  - 30MHz
  - 60MHz:wire==1
```
In this case, the *combination* `wire==2,speed==60MHz` will not exist in *testcases*.
> A *constraint* is also a *condition*.

## 2 Usage
The `tclib` binary accepts at most 2 arguments.
```bash
tclib yamlite-in [testcases-out]
```
If the optional `testcases-out` is not given, *testcases* will be printed to standard output.

***

# tclib
&copy; 2019 ricynlee
MIT许可证

**测试用例批量生成工具**

## 1 测试用例集描述文件
批量自动测试可以用于全面测试产品特性.为了方便自动测试,可以使用yamlite文件描述测试用例集.
下面给出一个例子:测试单板计算机的boot功能.测试用例集描述如下.
```yamlite
flash:
  - nand
  - nor
wire:
  - 1
  - 2
  - 4
  - 8
speed:
  - 30MHz
  - 60MHz
encrypted:
  - yes
  - no
encryption:
  - aes128
  - aes256
```
为了简化后文描述,这里引入一些名词.

名词 | 例子 | 注释
-|-|-
key | `flash`, `encrypted` | 由字(大小写敏感),数字和下划线组成
value | `4`, `30MHz`, `aes128` | 视为字符串/文本
feature | `speed=30MHz`, `encrypted=yes`
combination | `flash=nand,wire=1,encrypted=yes`
testcase | 完整的*combination*,如`flash=nand,wire=1,speed=30MHz,encrypted=yes,encryption=aes128`
condition | `0`(逻辑0), `flash==nand`, `wire!=4`, `flash==nor \|\| !(encrypted==yes && encryption==aes128)` | 在`==`(或`!=`)之前务必为*key*,之后是**有效的***value*

**tclib**解析输入的yamlite文件, 从各个*key*中选择一个*value*,串接这些*feature*从而得到*testcase*.
**tclib**枚举所有可行的组合.对于上面的yamlite文件,共有64个*testcase*.

### 1.1 Dependencies
前一节的例子中含有一个*combination*即`encrypted=no,encryption=aes128`.这个组合并不合理:若`encrypted=no`, 则根本不应有`encryption`这个*key*.对于这种情况,可以为*key*添加*dependency*. 
```yamlite
encryption:encrypted==yes
  - aes128
  - aes256
```
上面的例子中,若`encrypted==no`则跳过`encryption`.
> 这里的*dependency*是*condition*.一个*condition*由形如`&&`的操作符,和形如`flash==nand`的操作数组成.注意`==`和`!=`并非操作符.

### 1.2 Constraints
可能出现这种情况:仅当`wire==1`时才支持`speed==60MHz`.此时可为`speed` *key*的值添加*constraint*,如下所示.
```yamlite
speed:
  - 30MHz
  - 60MHz:wire==1
```
这样,*testcase*中不会出现`wire==2,speed==60MHz`这个*combination*.
> 这里的*constraint*也是*condition*.

## 2 用法
应用程序`tclib`可以接受至多两个参数.
```bash
tclib yamlite-in [testcases-out]
```
如果`testcases-out`未给出,*testcase*会被显示到标准输出.
