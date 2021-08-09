#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include<iostream>
#include<stdexcept>
#include<cstdlib>

#include <vector>
#include <optional>
#include <set>
#include <cstdint> // Necessary for UINT32_MAX
#include <algorithm> // Necessary for std::min/ std::max

#include <fstream> //To load our -o file of shader TO DO: It`s better to do it in a absolute class

class BillionTrangleAPP {
public:

	BillionTrangleAPP() {
		shaderPath = "bin/Debug/shaders";
	}

	BillionTrangleAPP(char *argv) {
		int i = 0;
		while (argv[i] != '\0')
			i++;

		for (int index = i;; index--) {
			if (argv[index] == '\\') {
				argv[index + 1] = '\0';
				break;
			}
		}

		shaderPath = argv;
	}

	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

public:
	const uint32_t VL_WIDTH = 1260;
	const uint32_t VL_HEIGHT = 640;
	const char* VL_NAME = "Vulkan Billion Trangle";
	std::string shaderPath;

private:
	GLFWwindow *window;
	VkInstance instance;
	VkSwapchainKHR  swapChain;
	std::vector<VkImage> swapChainImages;// The handle of VkImage
	VkFormat swapChainImageFormat;       // The format we choose fot swapChain
	VkExtent2D swapChainExtent;			 // The format we choose fot swapChain
	VkSurfaceKHR surface;

	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkCommandBuffer> commandBuffers;
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;	// To specified the unifrom value in shader
	
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFrameBuffers;
	VkCommandPool commanPool;

	VkSemaphore imageAvailableSemaphore;	//signal
	VkSemaphore renderFinishedSemaphore;

	VkPhysicalDevice phyDevice = VK_NULL_HANDLE;
	VkDevice device;// The logic Device 

	VkQueue graphicsQueue;
	VkQueue presentQueue; // The queue which surpport windows-surface

	const std::vector<const char*>deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	// The format of swapChain
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	// Use to find the queueIndices
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value();
		}
	};


private:
	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, false);

		window = glfwCreateWindow(VL_WIDTH, VL_HEIGHT, VL_NAME, nullptr, nullptr);


	}

	void initVulkan()
	{
		createInstance();
		createSurface();
		pickPhyDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createCommandBuffers();
		creteSemaphores();
	}

	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {

			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];

			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;// Use the default value

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create image views !!!");
		}

	}

	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(phyDevice);//获取支持的格式等信息

		//从获取的信息里选择较为匹配的mode/format
		VkSurfaceFormatKHR sF = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR pM = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		// Define the min num image in our swap chain
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount +1 ;

		if (swapChainSupport.capabilities.maxImageCount > 0 &&
			imageCount > swapChainSupport.capabilities.maxImageCount)// 当Count 大于允许的最大值时，置为最大值
			imageCount = swapChainSupport.capabilities.maxImageCount;

		// Fill the swapChain struct
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = sF.format;
		createInfo.imageColorSpace = sF.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		/*
		// The imageArrayLayers specifies the amount of layers each image consists of. This is always 1 unless 
		// you are developing a stereoscopic 3D application. The imageUsage bit field specifies what kind of 
		// operations we'll use the images in the swap chain for. In this tutorial we're going to render directly 
		// to them, which means that they're used as color attachment. It is also possible that you'll render images 
		// to a separate image first to perform operations like post-processing. In that case you may use a value like 
		// VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to transfer the rendered image to a swap chain image.
		*/

		QueueFamilyIndices indices = findQueueFamilies(phyDevice);
		uint32_t queueFamIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // 并发模式
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;  // 独占模式，享有最佳的性能
			createInfo.queueFamilyIndexCount = 0;		// Optional
			createInfo.pQueueFamilyIndices = nullptr;	// Optional
		}

		// You can apply some Transform by changing this
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform; 

		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;// 指定是否使用alpha通道与窗口系统中的其他窗口混合

		createInfo.presentMode = pM;
		createInfo.clipped = VK_TRUE; // We don`t care the color of a pixel obscured, use clip to get the best performance

		// If your window size or another has changed, you should create your swapChain from 0. Thus we should 
		// figure out the refence of the old chain. We won`t use it until we touch the part. 
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
			throw std::runtime_error("failed to create swap chain !");

		// Get the handle of the image in swapChain
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = sF.format;
		swapChainExtent = extent;
	}

	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface !!!");

	}

	void createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(phyDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t q : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo que{};
			que.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			que.queueFamilyIndex = q;
			que.queueCount = 1;
			que.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(que);
		}

		// If you want to do more interesting things, you`ll change the struct
		VkPhysicalDeviceFeatures devFur{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();


		createInfo.pEnabledFeatures = &devFur;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.enabledLayerCount = 0; // It will be ignore by the new achieve

		if (vkCreateDevice(phyDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0 /* Index */, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	void pickPhyDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (!deviceCount)
			throw std::runtime_error("Failed to find GPUs with Vulkan support ! /Actually maybe you don`t plug your Computert into your GPUs");
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		
		// To choose the GPUs in isDevicesSuitable. If you want to make a hack for different device/phones/computers, you`d like to change it in that function.
		for (const auto& device : devices) {
			if (isDevicesSuitable(device)) {
				phyDevice = device;
				break;
			}
		}

		if (phyDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU !!!");
		}
;	}

	bool isDevicesSuitable(VkPhysicalDevice device) {

		VkPhysicalDeviceProperties devProp;
		VkPhysicalDeviceFeatures devFur;
		vkGetPhysicalDeviceProperties(device, &devProp);
		vkGetPhysicalDeviceFeatures(device, &devFur);

		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extenstionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extenstionsSupported) {
			SwapChainSupportDetails swap = querySwapChainSupport(device);
			swapChainAdequate = !swap.formats.empty() && !swap.presentModes.empty();
		}

		return indices.isComplete() && extenstionsSupported && swapChainAdequate;

		return devProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && devFur.geometryShader;
	}

	void createInstance() {
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = VL_NAME;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 1);//Vulkan version in my Computer
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 1);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Get the platExtension from GLFW
		uint32_t glfwEntensionCount = 0;

		createInfo.ppEnabledExtensionNames = glfwGetRequiredInstanceExtensions(&glfwEntensionCount);
		createInfo.enabledExtensionCount = glfwEntensionCount;
		
		createInfo.enabledLayerCount = 0;

		//CheckExtension();// Annotation it , and use it when you want to check the surpport SDK 

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create intance !!!");
		}

	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();
		}
	}

	void cleanup() {
		vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);

		vkDestroyCommandPool(device, commanPool, nullptr);

		for (auto framebuffer : swapChainFrameBuffers)
			vkDestroyFramebuffer(device, framebuffer, nullptr);

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		for (auto imageView : swapChainImageViews)
			vkDestroyImageView(device, imageView, nullptr);

		vkDestroySwapchainKHR(device, swapChain, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}


	// The render pass Part
	void createRenderPass() {

		// Attachment Part
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;		//保留附件的现有内容
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;		//渲染的内容将存储在内存中，以后可以读取
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// SubPass Part
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;


		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;


		VkSubpassDependency dependeny{};
		dependeny.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependeny.dstSubpass = 0;
		dependeny.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependeny.srcAccessMask = 0;
		dependeny.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependeny.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependeny;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) 
			throw std::runtime_error("Failed to create render pass !!!");
		
	}

	// The pipeLine Part
	void createGraphicsPipeline()
	{
		auto vertShaderCode = readFile(shaderPath + "shaders/vert.spv");
		auto fragShaderCode = readFile(shaderPath + "shaders/frag.spv");

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);


		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // 告诉vulkan这个将会在哪个流水阶段使用。
		vertShaderStageInfo.module = vertShaderModule;	// 指定包含代码的着色器模块
		vertShaderStageInfo.pName = "main";				// 指定要调用的函数名

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // 告诉vulkan这个将会在哪个流水阶段使用。
		fragShaderStageInfo.module = fragShaderModule;	// 指定包含代码的着色器模块
		fragShaderStageInfo.pName = "main";				// 指定要调用的函数名


		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};


		// The FIXED PART..........................................

		// 指定从顶点缓冲区读取data的格式和顶点属性设置
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;  //Optinal
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;	// Optional

		// 描述顶点将绘制成哪种图元以及是否启用图元复用
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// 视口和剪裁
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0,0 };
		scissor.extent = swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// 光栅化
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE; // True means Geometry will never pass the Rasterize
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // 设置几何生成模式: FILL 填充  LINE 线框  POINT 点 ~~使用除填充以外的任何模式都需要启用 GPU 功能
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;	// 指定面剔除类型
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // 指定正面的顶点顺序(逆时针/顺时针)
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;		// Opional
		rasterizer.depthBiasClamp = 0.0f;               // Opional
		rasterizer.depthBiasSlopeFactor = 0.0f;			// Opional

		// 多重采样(暂时禁用)
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;							// Opional
		multisampling.pSampleMask = nullptr;							// Opional
		multisampling.alphaToCoverageEnable = VK_FALSE;					// Opional
		multisampling.alphaToOneEnable = VK_FALSE;						// Opional

		// To Do: 深度和模板测试

		// 颜色混合/Color Blender
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};	//每个帧缓冲区的颜色混合设置
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;				// Opional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;			// Opional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;						// Opional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;				// Opional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;			// Opional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;						// Opional
			
		VkPipelineColorBlendStateCreateInfo colorBlending{};	//全局混合设置
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;		// Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;			// Optional
		colorBlending.blendConstants[1] = 0.0f;			// Optional
		colorBlending.blendConstants[2] = 0.0f;			// Optional
		colorBlending.blendConstants[3] = 0.0f;			// Optional

		// 动态state 填充以下结构可以无需重新创建pipeline而动态调整
		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		// 管道布局/Pipeline Layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;				// Optional
		pipelineLayoutInfo.pSetLayouts = nullptr;			// Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0;		// Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr;	// Optional

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create pipeline layout !!!");


		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;				// Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;					// Optional

		pipelineInfo.layout = pipelineLayout;

		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		//设置派生管道
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;		// Optional
		pipelineInfo.basePipelineIndex = -1;					// Optional


		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
			throw std::runtime_error("Failed to creae graphic pipeline !!!");

		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	// The FrameBuffer Part
	void createFramebuffers(){
		swapChainFrameBuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++){
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFrameBuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create framebuffer !!!");

		}

	}

	// The command Pool Part
	void createCommandPool() {

		QueueFamilyIndices queueFamilIndices = findQueueFamilies(phyDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilIndices.graphicsFamily.value();
		poolInfo.flags = 0;		// Optional

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commanPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create command pool !!!");

	}

	void createCommandBuffers() {
		commandBuffers.resize(swapChainFrameBuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commanPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers !!!");

		for (size_t i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;		// Optional
			beginInfo.pInheritanceInfo = nullptr; //Optional

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
				throw std::runtime_error("Failed to begin recording command buffer !!!");

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFrameBuffers[i];

			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = swapChainExtent;

			VkClearValue clearColor = { {{0.2f, 0.3f, 0.4f, 1.0f}} };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to record command buffer !!!");
		}

		
	}

	// Create semaphores
	void creteSemaphores() {
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS
			|| vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
			throw std::runtime_error("Failed to create semaphores");
	}

	// Frame Rander Part
	void drawFrame() {
		uint32_t imageIndex;
		vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
			throw std::runtime_error("Fialed to submit draw command buffer !!!");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		presentInfo.pResults = nullptr;

		vkQueuePresentKHR(presentQueue, &presentInfo);

	}

private:

	// Find a enumType Queue Family
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t quFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &quFamilyCount, nullptr);

		

		std::vector<VkQueueFamilyProperties> queueFamilies(quFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &quFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& q : queueFamilies) {

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
				indices.presentFamily = i;// Find the queue which surpport windows-surface

			if (q.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
			i++;
			if (indices.isComplete())
				break;
		}

		return indices;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector <VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		return requiredExtensions.empty();
	}

	// To fill the struct
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	// Chose the colorSpace format
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& av : availableFormats) {
			if (av.format == VK_FORMAT_B8G8R8A8_SRGB && av.colorSpace ==
				VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return av;
		}

		return availableFormats[0];
	}

	// Chose the presentation mode
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& avPreModes)
	{
		for (const auto& av : avPreModes) {
			if (av == VK_PRESENT_MODE_MAILBOX_KHR) {
				return av;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR; // fisrt in first out
	}

	// Chose the swap region in Screen
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capab)
	{
		if (capab.currentExtent.width != UINT32_MAX){
			return capab.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capab.minImageExtent.width,
				capab.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capab.minImageExtent.height,
				capab.maxImageExtent.height);

			return actualExtent;
		}
	}

	// To load the -o file of compiled shader
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file!(almost the compiled shader binary file");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;

	}

	// To packet binary code in a VkShaderModule object
	VkShaderModule createShaderModule(const std::vector<char>& code) {

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*> (code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			throw std::runtime_error("Failed to create shader module !!! ");

		return shaderModule;
	}

	// Check Extersion
	void CheckExtension() {
		// Have Done: Create a function to check if all extension in the VK_ExtensionProperties
		uint32_t count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		std::vector<VkExtensionProperties> extensions(count);
		vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
		std::cout << "VK surpport: "<< count <<"\n";
		for (const auto& extension : extensions)
			std::cout << "\t" << extension.extensionName << "\n";

		uint32_t glfwCount = 0;
		const char** ex = glfwGetRequiredInstanceExtensions(&glfwCount);
		std::cout <<"GLFW Request: " << glfwCount << "\n";
		while (--glfwCount<-1) {
			std::cout << "\t"<< *ex << "\n";
			*ex++;
		}

		//TO DO: add a vector to save all KHR you want to check instead of glfwRequired;
		uint64_t tmp;
		int Hsh[1000];
		for (int i = 0; i < count; i++) {
			char* tp = extensions[i].extensionName;

			tmp = tm_murmur_hash_64a(tp, sizeof(tp), 0);
			Hsh[tmp%1000] = 1;
		}

		ex = glfwGetRequiredInstanceExtensions(&glfwCount);
		for (int i = 0; i < glfwCount; i++) {
			tmp = tm_murmur_hash_64a(*(ex + i), sizeof(*(ex+i)), 0);
			if (Hsh[tmp % 1000] != 1) {
				std::cout << "Not surppot: " << *(ex + i) << "\n";
			}
		}
		std::cout << "All surport !";
	}
	static inline uint64_t tm_murmur_hash_64a(const void* key, uint32_t len, uint64_t seed)
	{
		const uint64_t m = 0xc6a4a7935bd1e995ULL;
		const int r = 47;

		uint64_t h = seed ^ (len * m);

		const uint64_t* data = (const uint64_t*)key;
		const uint64_t* end = data + (len / 8);

		while (data != end)
		{
			uint64_t k;
			memcpy(&k, data++, sizeof(k));

			k *= m;
			k ^= k >> r;
			k *= m;

			h ^= k;
			h *= m;
		}

		const unsigned char* data2 = (const unsigned char*)data;

		switch (len & 7) {
		case 7:
			h ^= (uint64_t)(data2[6]) << 48;
		case 6:
			h ^= (uint64_t)(data2[5]) << 40;
		case 5:
			h ^= (uint64_t)(data2[4]) << 32;
		case 4:
			h ^= (uint64_t)(data2[3]) << 24;
		case 3:
			h ^= (uint64_t)(data2[2]) << 16;
		case 2:
			h ^= (uint64_t)(data2[1]) << 8;
		case 1:
			h ^= (uint64_t)(data2[0]);
			h *= m;
		};

		h ^= h >> r;
		h *= m;
		h ^= h >> r;

		return h;
	}
};



int main(int argc, char** argv) {

	BillionTrangleAPP app(*argv);

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;


}