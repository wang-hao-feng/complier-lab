# 哈工大编译原理实验

## 前言
这是哈工大的编译原理的实验，用的是《编译原理实践与指导教程》这本书
由于砍了学时，实验只做了前三个，第三个还没优化中间代码，所以只给出了中间代码生成之前的一些建议，假期如果把没做的补上了的话也会把建议添加进来
这次实验前后关系及其紧密，因此前一个设计如果出问题了，后一个实验就会举步维艰，甚至需要重构代码，所以我在这给出的是方便后面实验的一些建议（当然我自己没来得及搞，因为实验就给了两周的时间，而且还有数据库的实验一块来，所以实际上只有一周）
这次实验个人觉得设计得比较好和比较差的地方我都会直接写出来，代码可读性不是特别好（但是自己能看懂），因此我重点写一写思路，争取可以在不看代码的情况下学会怎么写

## Lab1 词法分析和语法分析
词法分析没啥好说的，语法分析按照指导书写的去建树，这棵树对后面的实验及其友好，因为bison不容易传继承属性，而建树后自顶向下传继承属性只要多设置几个参数即可
我建树是用二叉树模拟了多叉树，因为子节点数量不定，所以封装建树过程的时候使用了变长参数表，这样在debug的时候只用改建树的函数即可，每个表达式那里调用建树函数就行
建议弄一个文件为每个表达式设置一个宏定义，并在建树的时候使用了哪个表达式添加到节点中，这样一可以增加可读性，二为之后的判断节省了时间

## Lab2 语义分析
虽然指导书说可以直接在bison上实现语义分析（非递归的），但要是真这么写了，后面debug和中间代码生成就会很难搞了，逻辑容易乱，而且如果要实现如识别结构体的功能，写在bison里面就有很多继承属性不好传的问题，因此还是递归地搞吧
递归的搞法建议为每个非终结符写一个分析函数，然后再为每个生成式写一个，这样后面中间代码生成如果想要插入到里面也很方便
递归的具体过程是，先进入非终结符的分析函数，根据节点里面存的表达式信息（实验一我建议加进去的）选择调用哪个产生式的分析函数
最好自顶向下开始弄，首先写定义相关的函数，这样可以尽早确定数据结构
数据结构方面，类型表达式按照指导书上的链表存即可。建议将每个数组元素的大小存到数组的类型表达式中，还有结构体中每个域相对首地址的偏移量，他们俩自己的大小也要存好，这样方便中间代码那用。数组和结构体的分析完全是可以递归来搞的，所以不是特别难，就是要想清楚每个非终结符返回了什么
像栈这类常用的数据结构，因为实验很多地方都可能用到，而且其需要存的数据类型基本不同，因此需要一种更通用的方式来写，从而实现代码复用。可以使用void *来实现针对任意类型的存储，而具体是什么类型交由调用者决定；相关函数的返回值也返回void *，调用者只要强转成对应的类型即可。可以看下我的TypeExp.h和TypeExp.c中的怎么使用的
而像map这样的结构，它还需要去比较任意两个对象是否相等，往往需要重载函数，这里可以用函数指针实现重载的功能，在每次建立结构体的时候传入一个函数指针，而在实现的时候就调用这个函数指针即可。我因为没这么干，map的key只能为int类型，实际上配合使用void *和函数指针可以实现任何类型的key
如果想参考我的代码，建议将不看的地方全部折叠起来，因为我是用if语句来判断使用了什么表达式的，所以整体看上去会很乱，但只要把分支理清楚了，感觉还不是那么难看懂（后面中间代码那时候我还能看懂在干嘛，甚至还能修改）

## Lab3 中间代码生成
如果你lab2是递归写的，这里同样有两种搞法：一种是和lab2一样，重新从根节点开始分析，写中间代码；另一种是直接在分析的函数里面写中间代码（我选的后者，因为前者还得重新写部分分析，没那个时间）
如果你也打算用后者，那按照我上面建议的办法来弄，可以少遇见很多问题（比如我遇到的问题有中间代码生成了两遍，因为我某些生成单独拿出来作为了函数，分析时调用生成的函数，而生成又需要分析子节点，这样生成和分析都生成了子节点的中间代码；而按照我lab2的建议实现的话，可以直接将生成嵌入到分析的函数里面而不显得代码臃肿）每个产生式只需要关系生成自身的中间代码，这样基本可以照抄指导书里面的代码了
在类型表达式里面多添加一个域，用来存放中间代码中这个变量的变量名，这样要使用的时候直接查符号表就能马上知道中间代码中哪个变量对于此变量
在生成的中间代码的时候，利用lab2保存的大小以及偏移量的关系，可以很计算出数组元素以及结构体域的地址
中间代码结构体方面我说不好，因为没写优化和后面的目标代码生成，所以不知道哪个好哪个不好，后面要是写了可能会补上