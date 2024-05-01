## Veach场景的Monte Carlo Path Tracing

Monte_Carlo_Path_Tracing for CG course 2023 zju

#### 一、渲染方程

1. 由入射光线方向$$\omega_i$$​的视角
   $$
   L_o(p,\omega_o)=L_e(p,\omega_o)+\int_{\Omega^+}{L_i(p,\omega_i)f_r(p,\omega_i,\omega_o)(n \cdot \omega_i)d\omega_i}\tag{1}
   $$
   
2. 由光源平面采样点$\mathbf{x^\prime}$的视角
   $$
   \begin{eqnarray} L_o(x,\omega_o) &=& \int_{\Omega^+}{L_i(x,\omega_i)f_r(x,\omega_i,\omega_o)cos\theta d\omega_i} \nonumber \\                                  &=& \int_A{L_e(\mathbf{x^\prime}\rightarrow\mathbf{x})f_r(x,\omega_i,\omega_o)}\frac{cos\theta cos\theta^{\prime}}{\Vert \mathbf{x^{\prime}}-\mathbf{x}\Vert^2}dA \tag{2} \end{eqnarray}
   $$
   <img src="C:\Users\luotong\Desktop\图形学\exp2汇总\image-20240430124916283.png" alt="image-20240430124916283" style="zoom:50%;" />

   以上图片及公式部分参考于Games101课程[^1]

3. 

#### 二、Phong模型的brdf函数

​	
$$
\begin{eqnarray} f_r(\mathbf{x}, \omega_i, \omega_r) &=& f_{r,d}(\mathbf{x}, \omega_i, \omega_r) + f_{r,s}(\mathbf{x}, \omega_i, \omega_r) \nonumber \\ &=& \rho_d \frac{1}{\pi} + \rho_s \frac{n+1}{2\pi}\cos^n \alpha \tag{3} \end{eqnarray}
$$

式$(3)$参考于[^2]

- $\rho_d$是diffuse反射系数，$\rho_s$是specular反射系数，$\alpha$是$\omega_r$的完美镜面反射$R$与入射光线$\omega_i$之间的夹角，$n$​​是Phong模型exponent参数（shiness）
- 注意：$\theta>\frac{\pi}{2}$时，第一项会取$0$。$\alpha>\frac{\pi}{2}$时，第二项会取$0$。

#### 三、光线采样方案：

为了采用MonteCarlo算法求式$(2)$的积分，需要对半球面积分域$\Omega^+$进行光线采样。为使MonteCarlo积分估计的方差较小，采样方法的概率密度函数形式需要尽可能与被积函数形式一致。而式$(2)$的被积函数可划分为三项的乘积：光源项$L_i(x,\omega_i)$，brdf项$f_r(x,\omega_i,\omega_o)$，几何项$cos\theta$（或者$\frac{cos\theta cos\theta^{\prime}}{\Vert x^{\prime}-x^2\Vert}$​）。因此可以分别以光源，brdf为光线采样的概率密度函数基准。

##### A.光源采样

本实验的光源采样方案有两种：

1. 光源均匀采样：将采样划分为3个阶段，偏向采样Radiance高的光源及面积大的光源。

   具体步骤：

   - 按照光源radiance作为概率权重，随机采样一类Radiance的光源。
   - 在该类光源三角形中，以三角形面积作为概率权重，随机采样一个三角形。
   - 在该三角形中均匀采样一个点，作为光源采样的结果。在一个光源三角形内均匀采样的细节：先在2d平面上${\{(\beta,\gamma)|\beta\geq0,\gamma\geq 0,\beta+\gamma\leq 1\}}$的三角形区域中均匀采样，然后仿射变换到空间中的光源三角形。重心坐标$(\alpha,\beta,\gamma)$的使用参考本课程ppt[^8]。

   （备选采样方案：假设$p(\omega_i) \propto L_i(x,\omega_i)dA$，$p(R) = \frac{RS_R}{\sum{rS_r}}$，选定Radiance R以后，具体三角形按照面积为权重采样。待补充）

2. **半球面的投影三角形上均匀采样**： 

   考虑光源项（和部分几何项），假设$p_1(\mathbf{x}) \propto L_e(\mathbf{x^\prime}\rightarrow\mathbf{x})\frac{cos\theta^{\prime}}{\Vert \mathbf{x^{\prime}}-\mathbf{x}\Vert^2},\mathbf{x}\in \cup{A_i}$，$A_i$是第i个光源三角形的点集。

   由积分变量替换可知，上式等价于$p_2(\omega_i)\propto L_e(\mathbf{x^\prime}\rightarrow\mathbf{x}),\enspace w_i\in \cup{A_i^\prime}$，$A_i^\prime$是第$i$个光源三角形径向投影(不考虑被遮挡)到半球面上所得球面三角形(Spherical Triangle)的点集。

   光源三角形投影到半球面上后得到球面三角形，在这些球面三角形上按面积均匀采样。在一个Spherical Triangle上均匀采样的细节参考于Arvo1995[^3]的5.2.1节

   采样$p_2(\omega_i)$的具体步骤：

   - 计算所有光源三角形在当前点$\mathbf{x^\prime}$的半球面$H^2$上的投影三角形，由单位向量$\mathbf{A},\mathbf{B},\mathbf{C}$刻画。同时计算相关中间变量$a,b,c,\alpha,\beta,\gamma$等，以及投影三角形的面积$\mathcal{A}$。 
   - 对于每个光源三角形，以光源Radiance乘以投影面积$L*\mathcal{A}$​作为概率权重，采样一个光源三角形。
   - 在这个光源三角形上按面积均匀采样出一个方向，作为光源采样的结果。

   此处可以进一步考虑假设$p_3(\omega_i)\propto L_e(\mathbf{x^\prime}\rightarrow\mathbf{x})cos\theta,\enspace w_i\in \cup{A_i^\prime}$该如何采样，待补充。

3. 以上两种采样方案都有使用Radiance*面积来作为光源三角形的概率权重，这里解释一下为何正确。

   假设$p(\omega) \propto L ,x\in\Omega,\Omega = \cup\Omega_i$，$L$在$\Omega_i$上是常数。

   那么$p(\omega) = cL$，由概率归一性，$c\int_\Omega Ld\omega=1$，则$c\sum_{k}{\int_{\Omega_k}{L_kd\omega}} = 1$，即$c=\frac{1}{\sum_{k}{L_k\int_{\Omega_k}{d\omega}}}$

   那么$P(\omega\in \Omega_i) = c\int_{\Omega_i}L_id\omega=\frac{L_i\int_{\Omega_i}d\omega}{\sum_{k}{L_k\int_{\Omega_k}{d\omega}}}$，其中$\int_{\Omega_i}d\omega$​在本实验中就是光源（球面）三角形的面积。

   

##### B.brdf采样

1. **Phong模型**

   考虑brdf项和几何项，假设$p_4(\omega_i)\propto f_r(\mathbf{x}, \omega_i, \omega_r) cos\theta =\rho_d \frac{1}{\pi}cos\theta + \rho_s \frac{n+1}{2\pi}\cos^n\alpha cos\theta，\omega_i \in H^2_{\mathbf{x},\mathbf{n}}$，$H^2_{\mathbf{x},\mathbf{n}}$是在$\mathbf{x}$处以$\mathbf{n}$为中心轴的正半球面。

   由于$\alpha$是$\omega_r$的完美镜面反射$R$与入射光线$\omega_i$之间的夹角， 直接对$p_4(\omega_i)$在$H^2_{\mathbf{x},\mathbf{n}}$上积分极为复杂[^5]，并且需要数值方法求逆。于是退求其次，丢掉specular部分的$cos\theta$权重， 假设$p_5(\omega_i)\propto \rho_d \frac{1}{\pi}cos\theta + \rho_s \frac{n+1}{2\pi}\cos^n\alpha,\enspace \omega_i \in H^2_{\mathbf{x},\mathbf{n}}$​，其精确积分仍然复杂。

   于是将$\omega_i$所属空间扩大，使得$\omega_i \in H^2_{\mathbf{x},\mathbf{n}} \cup H^2_{\mathbf{x},\mathbf{R}}$。可得$p_6(\omega_i)=c (\rho_d \frac{1}{\pi}cos\theta + \rho_s \frac{n+1}{2\pi}\cos^n\alpha),\enspace \omega_i \in H^2_{\mathbf{x},\mathbf{n}} \cup H^2_{\mathbf{x},\mathbf{R}}$。

   对$p_6(\omega_i)$​积分：
   $$
   \begin{eqnarray}
   1 &=& \int_{H^2_{\mathbf{x},\mathbf{n}} \cup H^2_{\mathbf{x},\mathbf{R}}}{c (\rho_d \frac{1}{\pi}cos\theta + \rho_s \frac{n+1}{2\pi}\cos^n\alpha)}d\omega_i  \tag{4}\\
   1 &=& c(\rho_d\int_{H^2_{\mathbf{x},\mathbf{n}}}{ \frac{1}{\pi}cos\theta}d\omega_i + \rho_s \int_{H^2_{\mathbf{x},\mathbf{R}}}{\frac{n+1}{2\pi}\cos^n\alpha}) \tag{5}\\
   1 &=& c(\rho_d + \rho_s) \tag{6}\\
   c &=& \frac{1}{\rho_d + \rho_s}
   \end{eqnarray}
   $$
   式$(4)$到式$(5)$可以将积分域划分为一个交集，两个差集来细致讨论。注意式$(5)$到式$(6)$​积分归一性的证明参见本课程ppt[^4]。

   因此对$p_6(\omega_i)$的采样步骤如下：

   - 以$\rho_d,\rho_s$作为概率权重，随机选择采样diffuse分量或者specular分量。
   - 若选中diffuse分量，则从cos weighted hemisphere $H^2_{\mathbf{x},\mathbf{n}}$上采样一个方向作为brdf采样结果。
   - 若选中specular分量，则从Phong lobe $H^2_{\mathbf{x},\mathbf{R}}$上采样一个方向作为brdf采样结果。注意需要在世界坐标系中将坐标由$\mathbf{R}$为中心轴变换至以$\mathbf{n}$​为中心轴。

   Cos weighted hemisphere 采样和Phong lobe采样具体细节参考于本课程ppt[^4]。

   注意：可能会采样出指向$\mathbf{n}$的负半空间方向。

2. Blinn-Phong模型采样(待补充)

   可能会更简单，因为$p_4(\omega_i)$中的$\alpha$变为了$\theta$。

#### 四、Multiple Importance Sampling(MIS)

##### A.Multi-sample Model

假设：需要计算积分
$$
\int_{\Omega}{f(x)d\mu(x)} \tag{7}
$$
其中积分域$\Omega$，函数$f:\Omega \rightarrow \mathbb{R}$，测度$\mu$均已知。

- 现有$n$种积分域$\Omega$上不同的采样方法，其各自采样概率密度为$p_1,p_2...p_n$。

- 任意$x \in \Omega$，$p_i(x)$可以被计算。

##### B.Multi-sample estimator

$$
F=\sum_{i=1}^{n}{\frac{1}{n_i}\sum_{j=1}^{n_i}{w_i(X_{i,j})\frac{f(X_{i,j})}{p_i(X_{i,j})}}} \tag{8}
$$

$F$是对式$(7)$的估计，其中$X_{i,j}$是第$i$种采样方法的第$j$个样本，$n_i$是第$i$种采样方法的样本数量。$w_i$是第$i$种采样方法的权重函数，其满足如下要求：

- 当$f(x) \neq 0$时，$\sum_{i=1}^{n}{w_i(x)}=1$ 。
- 当$p_i(x)=0$时，$w_i(x) = 0$​。

因此$F$可以看作是对式$(9)$的MonteCarlo估计，其中第$i$求和项使用第$i$种采样方法计算：
$$
\int_{\Omega}{f(x)d\mu(x)} = \sum_{i=1}^{n}{\int_{\Omega}{w_i(x)f(x)d\mu(x)}} \tag{9}
$$
可以容易证明$F$​是无偏的。

##### C.Balance heuristic

$$
\hat{w}_i(x)=\frac{n_ip_i(x)}{\sum_k{n_k p_k(x)}} \tag{10}
$$

使用式$(10)$的估计量$F$​可以被证明具有较小的方差上界。

式$(7)\sim (10)$参考于Stanford CS348b课程 slides[^6]。

##### D.本实验案例

本次实验中，反射方程即为待计算积分。由光线采样方案可知，有两种sampling 方法，每种采样方法的样本数量$n_i=1$：

- 光源采样，概率密度$p_0$，样本方向$X_0$
- brdf采样 ，概率密度$p_1$，样本方向$X_1$

因此，由式$(7)\sim(10)$，对$L_o(x,w_o)$的估计量为:
$$
F=\frac{f(X_0)}{p_0(X_0)+p_1(X_0)}+\frac{f(X_1)}{p_0(X_1)+p_1(X_1)} \tag{11}
$$
其中$f(\omega)=L_i(\mathbf{x},\omega)f_r(\mathbf{x},\omega,\omega_o)cos\theta$

可以进一步尝试power heuristic[^6]（待补充）

#### 五、Russian Roulette

本实验的Russian Roulette参数设置：

P_RR：光线继续递归计算的概率。

光源采样+brdf采样：P_RR = 0.6

仅brdf采样：P_RR = 0.8

#### 六、光线求交及加速结构

光线求交：直线与三角形求交，参考于课程ppt[^8]。

本实验加速结构采用3维uniform网格：

BerkeleyCS184[^7]提到网格数量设置的Heuristic为#cells = 27 * #objs。本实验facet数量为$3092$，故设置网格数量$n_0= 100000$​。

采用3D DDA算法，bresenham没意义。核心在于判断是否达到下一个整数网格的边界。3维降2维，2维降1维。非整数起始点。

#### 七、核心算法及代码

1. 直接光照与间接光照：反射Radiance根据能量来源被划分为**直接光照**和**间接光照**，分别计算，然后求和。

   - 直接光照使用1个**光源采样**样本计算
   - 间接光照使用1个**brdf采样**样本计算
   - 若brdf采样样本方向射向光源三角形，本次间接光照为0。因为这部分Radiance已经由直接光照计算。
   - 光源三角形暂时只考虑自发光，不计算反射Radiance。
   - 非光源三角形没有自发光，只计算反射Radiance。

   ```c++
   //main.cpp
   //计算交点point处，沿着wo方向射出的Radiance。
   RadianceRGB shade(intersec_result point, vec wo);
   ```

   该算法核心框架参考于Games101课程[^9]

   **与Multi-sample estimator的关系：**

   *结论1：此处直接光照与间接光照的划分及算法，可以看作是对式$(8)$ 中$F$​估计量的实现。（具体的$w_i$函数见下方细节）*

   - 两种采样方法：光源采样$p_0$、brdf采样$p_1$

   - 假设当前着色点$\mathbf{x}$，法向$\mathbf{n}$。所有光源三角形径向投影(不考虑被遮挡)到正半球面$H^2_{\mathbf{x},\mathbf{n}}$上所得点集为$A^\prime$，$B^\prime = \{\mathbf{a}\in A^\prime |由\mathbf{x}到\mathbf{a}发出的光线可以不被遮挡地射到某个光源三角形\}$。可知$B^\prime \subseteq A^\prime$，且$A - B^\prime$中的点与光源三角形中间有遮挡。

     正半球面$H^2_{\mathbf{x},\mathbf{n}}$上所有点构成的集合记为$U$​。

     采样出的单位方向向量为$\omega_i$。

   - 两种采样方案的weight函数定义如下：
     $$
     \begin{equation}
     w_0(\omega_i) =\left\{
     \begin{aligned}
     	\quad&1 \qquad \qquad  \mathbf{x}+\omega_i \in B^\prime \\
     	\quad&0 \qquad \qquad  \mathbf{x}+\omega_i \in U-B^\prime
     \end{aligned}
     \right.
     \end{equation}
     $$

     $$
     \begin{equation}
     w_1(\omega_i) =\left\{
     \begin{aligned}
     	\quad&0 \qquad \qquad  \mathbf{x}+\omega_i \in B^\prime \\
     	\quad&1 \qquad \qquad  \mathbf{x}+\omega_i \in U-B^\prime
     \end{aligned}
     \right.
     \end{equation}
     $$

     容易验证以上$w_i$函数的设定满足Muti-sample estimator的两条要求，故可得到结论1。

   - 

2. 

[^1]:[GAMES101: 现代计算机图形学入门 (ucsb.edu)](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)
[^2]:[Assignment 3: Phong and Multiple Importance Sampling (mcgill.ca)](https://www.cim.mcgill.ca/~derek/ecse689_a3.html)
[^3]:Arvo J R. Analytic methods for simulated light transport[D]. Yale University, 1995.
[^4]:11-mc_ii_2023.pdf
[^5]:本人推了一周公式，还推错了。可能会有助于后续对Blinn-Phong brdf采样的分析。![](C:\Users\luotong\Desktop\图形学\exp2汇总\F99FD86F-39F2-45E6-8527-E4A0757D2895_L0_001.JPG)
[^6]:[veach-chapter9.pdf (stanford.edu)](https://graphics.stanford.edu/courses/cs348b-03/papers/veach-chapter9.pdf)
[^7]:[10-acceleration (berkeley.edu)](https://cs184.eecs.berkeley.edu/public/sp21/lectures/lec-10-ray-tracing-acceleration/lec-10-ray-tracing-acceleration.pdf)
[^8]:8-ray_casting_2023.pdf
[^9]:[GAMES101_Lecture_16 (ucsb.edu)](https://sites.cs.ucsb.edu/~lingqi/teaching/resources/GAMES101_Lecture_16.pdf)