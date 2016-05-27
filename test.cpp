#include <vector>
#include <array>
#include <stdio.h>
#include <string.h>
#include <iomanip>
#include <vulkan/vulkan.h>
#include <iostream>
#include <chrono>

using namespace std;

#define BUFFER_SIZE		10240
#define INCREMENT_PASSES	50000
#define RUNS			5

void printDeviceProperties(int count, VkPhysicalDeviceProperties& pros) {
	cout << "---------------------- " << count << " ----------------------" << endl;
	cout << "apiVersion :" << pros.apiVersion << endl;
	cout << "driverVersion : " << pros.driverVersion << endl;
	cout << "vendorID : " << pros.vendorID << endl;
	cout << "deviceID : " << pros.deviceID << endl;
	cout << "deviceType : " << pros.deviceType << endl;
	cout << "deviceName : " << pros.deviceName << endl;
	//cout << "pipelineCacheUUID : " << pros.pipelineCacheUUID << endl;
	//cout << "limits : " << pros.limits.;
	//cout << "sparseProperties : " << pros.sparseProperties;
	cout << "---------------------- " << count << " ----------------------" << endl;
}

void printDeviceQueueFamilyProperties(int count, VkQueueFamilyProperties& pros) {
	cout << "---------------------- " << count << " ----------------------" << endl;
	cout << "queueFalgs : " << pros.queueFlags << endl;
	cout << "queueCount : " << pros.queueCount << endl;
	cout << "timestampValidBits : " << pros.timestampValidBits << endl;
	cout << "---------------------- " << count << " ----------------------" << endl;
}

void printPhysicalDeviceMemoryProperties(VkPhysicalDeviceMemoryProperties memoryProperties) {
	cout << "memoryTypeCount : " << memoryProperties.memoryTypeCount << endl;
	cout << "vkMemoryType : " << memoryProperties.memoryTypes << endl;
	cout << "memoryHeapCount : " << memoryProperties.memoryHeapCount << endl;
	cout << "memoryheaps : " << memoryProperties.memoryHeaps << endl;
}

char *readBinaryFile(const char *filename, size_t *psize)
{
	long int size;
	size_t retval;
	void *shader_code;

	FILE *fp = fopen(filename, "rb");
	if (!fp) return NULL;

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	shader_code = malloc(size);
	retval = fread(shader_code, size, 1, fp);
	cout << "readBinaryFile : " << (retval == 1) << endl;

	*psize = size;

	return (char*)shader_code;
}

VkShaderModule loadShader(const char *fileName, VkDevice device, VkShaderStageFlagBits stage)
{
	size_t size = 0;
	const char *shaderCode = readBinaryFile(fileName, &size);
	cout << "VkShaderModule loadShader : " << (size > 0) << endl;

	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo moduleCreateInfo;
	VkResult err;

	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;

	moduleCreateInfo.codeSize = size;
	moduleCreateInfo.pCode = (uint32_t*)shaderCode;
	moduleCreateInfo.flags = 0;
	err = vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule);
	cout << "VkShaderModule loadShader : " << (!err) << endl;

	return shaderModule;
}

VkPipelineShaderStageCreateInfo loadShader(const char * fileName, VkShaderStageFlagBits stage, VkDevice& device)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.module = loadShader(fileName, device, stage);
	shaderStage.pName = "main"; // todo : make param
	cout << "VkPipelineShaderStageCreateInfo loadShader : " << (shaderStage.module != NULL) << endl;
	//shaderModules.push_back(shaderStage.module);
	return shaderStage;
}

VkBool32 getMemoryType(VkPhysicalDeviceMemoryProperties& prop, uint32_t typeBits, VkFlags properties, uint32_t * typeIndex)
{
	for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((prop.memoryTypes[i].propertyFlags & properties) == properties)
			{
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}


int main() {
	VkApplicationInfo app_info;
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	app_info.pApplicationName = "vulkanTest";
	app_info.applicationVersion = 1;
	app_info.pEngineName = "vulkanTest";
	app_info.engineVersion = 1;
	app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);
	VkInstanceCreateInfo inst_info;
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = NULL;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledLayerCount = 0;
	inst_info.ppEnabledLayerNames = NULL;
	inst_info.enabledExtensionCount = 0;
	inst_info.ppEnabledExtensionNames = NULL;

	VkResult err;
	VkInstance instance;
	err = vkCreateInstance(&inst_info, NULL, &instance);
	cout << "vkCreateInstance err : " << err << endl;

	uint32_t gpu_count;
	err = vkEnumeratePhysicalDevices(instance, &gpu_count, NULL);

	VkPhysicalDevice* pDeviceVector = new VkPhysicalDevice[gpu_count];
	cout << "vkEnumeratePhysicalDevices : " << err << ", " << gpu_count << ", " << pDeviceVector[0] << endl;

	err = vkEnumeratePhysicalDevices(instance, &gpu_count, pDeviceVector);
	cout << "vkEnumeratePhsicalDevice : " << err << ", " << gpu_count << ", " << pDeviceVector[0] << endl;


	VkPhysicalDeviceProperties properties;
	for (int i = 0; i < gpu_count; i++) {
		vkGetPhysicalDeviceProperties(pDeviceVector[i], &properties);
		printDeviceProperties(i, properties);
	}

	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(pDeviceVector[0], &queueFamilyCount, NULL);
	cout << "vkGetPhysicalDeviceQueueFamilyProperties : " << queueFamilyCount << endl;;

	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	queueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pDeviceVector[0], &queueFamilyCount, queueFamilyProperties.data());
	cout << "vkGetPhysicalDeviceQueueFamilyProperties : " << queueFamilyCount << ", " << queueFamilyProperties.size() << endl;

	for (int i = 0; i < queueFamilyCount; i++) {
		printDeviceQueueFamilyProperties(i, queueFamilyProperties.at(i));
	}

	uint32_t queueIndex = 0;
	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(pDeviceVector[0], &queueCount, NULL);

	std::vector<VkQueueFamilyProperties> queueProps;
	queueProps.resize(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pDeviceVector[0], &queueCount, queueProps.data());

	for (queueIndex = 0; queueIndex < queueCount; queueIndex++)
	{
		if (queueProps[queueIndex].queueFlags & VK_QUEUE_COMPUTE_BIT)
			break;
	}

	std::array<float, 1> queuePriorities = { 1.0f };
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = queueIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = queuePriorities.data();

	cout << "VKDeviceQueueCreateInfo queueFamilyIndex : " << queueIndex << endl;

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.pEnabledFeatures = NULL;

	VkDevice device;
	err = vkCreateDevice(pDeviceVector[0], &deviceCreateInfo, nullptr, &device);

	cout << "createDevice : " << err << endl;

	VkPhysicalDeviceMemoryProperties dMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(pDeviceVector[0], &dMemoryProperties);
	printPhysicalDeviceMemoryProperties(dMemoryProperties);

	VkQueue queue = NULL;
	cout << "queue : " << queue << endl;
	vkGetDeviceQueue(device, queueIndex, 0, &queue);
	cout << "vkGetDeviceQueue : " << queue << endl;

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = queueIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkCommandPool cmdPool;
	err = vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool);
	cout << "vkCommandPool : " << err << ", " << cmdPool << endl;

	VkCommandBufferAllocateInfo cmdBufAllocateInfo;
	cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.commandPool = cmdPool;
	cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.commandBufferCount = 1;
	VkCommandBuffer computeCmdBuffer;
	err = vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &computeCmdBuffer);
	cout << "vkAllocateCommandBuffers : " << err << ", " << computeCmdBuffer << endl;

	VkMemoryAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = 0;
	allocateInfo.memoryTypeIndex = 0;
	VkBufferCreateInfo vBufferInfo;
	vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vBufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	vBufferInfo.flags = 0;
	vBufferInfo.size = sizeof(double) * BUFFER_SIZE;
	VkBuffer computeBuffer;
	err = vkCreateBuffer(device, &vBufferInfo, nullptr, &computeBuffer);
	cout << "vkCreateBuffer : " << err << ", " << computeBuffer << endl;
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, computeBuffer, &memoryRequirements);
	cout << "vkGetBufferMemoryRequirements : " << memoryRequirements.size << ", " << memoryRequirements.alignment << ", " << memoryRequirements.memoryTypeBits << endl;
	allocateInfo.allocationSize = memoryRequirements.size;
	getMemoryType(dMemoryProperties, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocateInfo.memoryTypeIndex);
	VkDeviceMemory deviceMemory;
	err = vkAllocateMemory(device, &allocateInfo, nullptr, &deviceMemory);
	cout << "vkAllocateMemory : " << err << ", " << deviceMemory << endl;

	//data copy
	void* deviceMemoryPtr = nullptr;
	err = vkMapMemory(device, deviceMemory, 0, vBufferInfo.size, 0, &deviceMemoryPtr);
	cout << "vkMapMemory : " << err << ", " << deviceMemoryPtr << endl;

	double* testData = new double[BUFFER_SIZE] {0};
	memcpy(deviceMemoryPtr, testData, sizeof(double)* BUFFER_SIZE);

	vkUnmapMemory(device, deviceMemory);

	err = vkBindBufferMemory(device, computeBuffer, deviceMemory, 0);
	cout << "vkBindBufferMemory : " << err << endl;

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
	VkDescriptorSetLayoutBinding layout1;
	layout1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layout1.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	layout1.binding = 0;
	layout1.descriptorCount = 1;
	layout1.pImmutableSamplers = NULL;
	setLayoutBindings.push_back(layout1);
	VkDescriptorSetLayoutCreateInfo descriptorLayout;
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = NULL;
	descriptorLayout.pBindings = setLayoutBindings.data();
	descriptorLayout.bindingCount = setLayoutBindings.size();
	VkDescriptorSetLayout computeDescriptorSetLayout = NULL;
	err = vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &computeDescriptorSetLayout);
	cout << "createDescriptorSetLayout : " << err << ", " << computeDescriptorSetLayout << endl;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = NULL;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &computeDescriptorSetLayout;
	VkPipelineLayout computePipelineLayout;
	err = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &computePipelineLayout);
	cout << "createPipelineLayout : " << err << ", " << computePipelineLayout << endl;

	std::vector<VkDescriptorPoolSize> poolSize;
	VkDescriptorPoolSize size1;
	size1.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	size1.descriptorCount = 1;
	poolSize.push_back(size1);
	VkDescriptorPoolCreateInfo descriptorPoolInfo;
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext = NULL;
	descriptorPoolInfo.poolSizeCount = poolSize.size();
	descriptorPoolInfo.pPoolSizes = poolSize.data();
	descriptorPoolInfo.maxSets = 1;
	VkDescriptorPool descriptorPool;
	err = vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool);
	cout << "createDescriptorPool : " << err << ", " << descriptorPool << endl;

	VkDescriptorSetAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.pSetLayouts = &computeDescriptorSetLayout;
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSet computeDescriptorSet;
	err = vkAllocateDescriptorSets(device, &allocInfo, &computeDescriptorSet);
	cout << "allocateDescriptorSets : " << err << ", " << computeDescriptorSet;

	std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets;
	VkWriteDescriptorSet writeSet1 = {};
	writeSet1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet1.pNext = NULL;
	writeSet1.dstSet = computeDescriptorSet;
	writeSet1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeSet1.dstBinding = 0;
	writeSet1.descriptorCount = 1;
	//writeSet1.dstArrayElement = 0;
	//writeSet1.pImageInfo = NULL;
	writeSet1.pTexelBufferView = NULL;
	VkDescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = computeBuffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = vBufferInfo.size;
	writeSet1.pBufferInfo = &descriptorBufferInfo;
	computeWriteDescriptorSets.push_back(writeSet1);
	vkUpdateDescriptorSets(device, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);

	VkComputePipelineCreateInfo computePipelineCreateInfo;
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.layout = computePipelineLayout;
	computePipelineCreateInfo.flags = 0;
	computePipelineCreateInfo.stage = loadShader("comp.spv", VK_SHADER_STAGE_COMPUTE_BIT, device);
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheCreateInfo.pNext = NULL;
	pipelineCacheCreateInfo.pInitialData = NULL;
	pipelineCacheCreateInfo.initialDataSize = 0;
	pipelineCacheCreateInfo.flags = 0;
	VkPipelineCache pipelineCache;
	err = vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache);
	cout << "vkCreatePipelineCache : " << err << ", " << pipelineCache << endl;
	VkPipeline pipeline;
	err = vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline);
	cout << "crateComputePipelines : " << err << ", " << pipeline << endl;
	if (err != VK_SUCCESS) {
		cout << "Fail createComputePipeline" << endl;
		return 0;
	}

	VkCommandBufferBeginInfo cmdBufInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = NULL;
	err = vkBeginCommandBuffer(computeCmdBuffer, &cmdBufInfo);
	cout << "VkCommandBufferBeginInfo : " << err << endl;

	vkCmdBindPipeline(computeCmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	vkCmdBindDescriptorSets(computeCmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &computeDescriptorSet, 0, 0);

	for(int j = 0; j < INCREMENT_PASSES; j++) {
		vkCmdDispatch(computeCmdBuffer, (BUFFER_SIZE / 256), 1, 1);
		vkCmdPipelineBarrier(computeCmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
	}

	err = vkEndCommandBuffer(computeCmdBuffer);
	cout << "vkEndCommandBuffer : " << err << endl;

	err = vkDeviceWaitIdle(device);
	cout << "vkDeviceWaitIdle : " << err << endl;

	VkSubmitInfo computeSubmitInfo;
	computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	computeSubmitInfo.pNext = NULL;
	computeSubmitInfo.commandBufferCount = 1;
	computeSubmitInfo.pCommandBuffers = &computeCmdBuffer;
	computeSubmitInfo.pWaitSemaphores = NULL;
	computeSubmitInfo.pWaitDstStageMask = NULL;
	computeSubmitInfo.pSignalSemaphores = NULL;
	computeSubmitInfo.waitSemaphoreCount = 0;
	computeSubmitInfo.signalSemaphoreCount = 0;

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
cout << fixed << setprecision(10);
  // double min = 99999, max = 0, temp = 0;
  for(int i = 0; i < RUNS; i++) {
    std::chrono::steady_clock::time_point submit = std::chrono::steady_clock::now();

  	err = vkQueueSubmit(queue, 1, &computeSubmitInfo, VK_NULL_HANDLE);

    cout << "vkQueueSubmit elapsed : " << chrono::duration_cast<chrono::milliseconds>(std::chrono::steady_clock ::now() - submit).count() << "ms" << std::endl;

    submit = std::chrono::steady_clock::now();

    err = vkQueueWaitIdle(queue);

    cout << "vkQueueWaitIdle elapsed : " << chrono::duration_cast<chrono::milliseconds>(std::chrono::steady_clock ::now() - submit).count() << "ms" << std::endl;
  }

	err = vkDeviceWaitIdle(device);
	cout << "vkQueueWaitIdle elapsed : " << chrono::duration_cast<chrono::milliseconds>(std::chrono::steady_clock ::now() - start).count() << "ms" << std::endl;
  // std::cout << "vkQueueSubmit elapsed : " << std::chrono::duration<double>(std::chrono::steady_clock ::now() - submit).count() << "ms" << std::endl;
  // cout << "min : " << min << ", max : " << max << endl;
	void* endDataPtr;
	err = vkMapMemory(device, deviceMemory, 0, vBufferInfo.size, 0, &endDataPtr);
	double* testData2 = new double[BUFFER_SIZE];
	memcpy(testData2, endDataPtr, sizeof(double)* BUFFER_SIZE);

	cout << "copy result : " << testData2[0] << endl;

	for(int i = 1; i < BUFFER_SIZE; i++) {
		if (testData2[i] != testData2[i - 1]) {
			cout << "Corruption at " << i << " : " << testData2[i] << " != " << testData2[i - 1] << endl;
		}
	}
	vkUnmapMemory(device, deviceMemory);

}
