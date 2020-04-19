# 作业 8 Shader 编程

刘紫檀 PB17000232

## 置换贴图+法向贴图

### 原理说明

法向贴图是通过让法向发生一些变化来改变光照的情况。

贴图中的 `rgb` 分别对应法线在切空间的三个基矢量 TBN（Tangent, Bitangent, Normal） 的分量大小（当然，需要归一化）。

具体来说，法向贴图要点如下：

1. （顶点着色器）计算 TBN 在世界坐标系中的值

   因为光照是在世界坐标系进行计算的，所以需要 TBN 在世界坐标系的值。

   现在已经知道了模型空间中的 T 和 N，只需要将 T 和 N 变换到世界坐标系，同时叉乘得到 B。

   T 的变换只需要左乘 model 变换矩阵即可。

   N 的变换则不然，为了保证变换后依然与平面垂直，需要用 `mat3(transpose(inverse(model)))` 这个变换矩阵，具体可以参考 [LearnOpenGL Basic Lighting](https://learnopengl-cn.github.io/02 Lighting/02 Basic Lighting/)。

2. （片段着色器）用 `rgb` 三分量分别乘以 TBN 三向量，归一化得到法向量在世界坐标系的值。

   当然，这个效果和 `normalize(vs_out.TBN * normal)` 是一样的。

置换贴图通过让顶点向法向移动一定距离来在物体上做出一些特殊效果。具体的说，只要给原顶点的位置 `aPos` 加上法向偏移 `aNormal * displacement[0] * displacement_coefficient` 即可。

> 由于一般置换贴图都是 r = g = b 的，所以取一个通道就够了。

### 效果展示

![image-20200420040431289](assets/image-20200420040431289.png)

![image-20200420040541604](assets/image-20200420040541604.png)

上图可以清晰的展示出法向贴图对光照的作用。

## 置换贴图降噪

### 原理说明

通过预计算的法向偏移，来让牛的身体变得平滑一些。

具体步骤如下：

1. 计算所有顶点的偏移量和法向点积的结果

   利用公式 $ \delta_i=p_i-\frac{1}{|N(i)|}\sum_{j\in N(i)}p_j $，其中临接点用 `getNeighbor` 函数获得

2. 获得最小的偏移和最大的偏移，根据其计算

   ```c
   displacement_bias = min_offset;
   displacement_scale = max_offset - min_offset;
   ```

3. 计算所有顶点对应纹理坐标处的 `displacementData`，与此同时构造 KDTree

4. 根据 KDTree 插值所有非对应点为顶点的纹理坐标处的偏移

   使用 `nanoflann` 库，并且取最近 10 个邻居的 displacement 按 $ L_2 $ 距离加权

5. 生成图片 + 绑定纹理

### 操作说明

新增一些按键：

```c++
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        displacement_lambda = -1;
        printf("current lambda: %lf\n", displacement_lambda);
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        displacement_lambda = 1;
        printf("current lambda: %lf\n", displacement_lambda);
    }

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        have_denoise = false;
        printf("Current denoise: %d\n", have_denoise);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        have_denoise = true;
        printf("Current denoise: %d\n", have_denoise);
    }
```

### 效果展示

![image-20200420032315489](assets/image-20200420032315489.png)

上图为未开启降噪的情况

![image-20200420032413867](assets/image-20200420032413867.png)

上图为开启降噪且设置 $ \lambda = - 1 $ 的情况，可以看到平滑了一些。

![image-20200420032626065](assets/image-20200420032626065.png)

上图为开启降噪（$\lambda = -1 $ ）的牛头。

![image-20200420032738621](assets/image-20200420032738621.png)

上图为关闭降噪的牛头。

![1_denoise_displacement_map](assets/1_denoise_displacement_map.png)

上图为没有进行插值情况下的牛头的置换纹理。

![1_denoise_displacement_map](assets/1_denoise_displacement_map-1587326392438.png)

上图为取最近 10 个邻居的 displacement 按 $ L_2 $ 距离加权插值的牛头的置换纹理。

## 阴影映射

### 原理说明

首先，以光源位置作为摄影机的位置，渲染一下当前图像，得到深度贴图；之后，用此深度贴图对比片段当前深度得到对应片元是否在阴影下，从而设置颜色。

关键部分：

1. 如何获得光源视角下的当前片段坐标，进而求出深度：

   由于光源和摄影机两者的世界坐标相同，则乘以光源的 view 和 projection 即可得到光源视角下的当前片段坐标（且为 NDC）。检查此坐标的深度值，并且和深度贴图中的值进行比较即可得到其是否被遮挡。

### BUGS

现在深度纹理似乎都是 1...比较神秘

导致一关灯就会全黑..

（就不放图啦）