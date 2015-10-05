/* WARNING: This is auto-generated file. Do not modify, since changes will
 * be lost! Modify the generating script instead.
 */
Move<VkInstance>					createInstance					(const PlatformInterface& vk, const VkInstanceCreateInfo* pCreateInfo);
Move<VkDevice>						createDevice					(const InstanceInterface& vk, VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo);
Move<VkDeviceMemory>				allocMemory						(const DeviceInterface& vk, VkDevice device, const VkMemoryAllocInfo* pAllocInfo);
Move<VkFence>						createFence						(const DeviceInterface& vk, VkDevice device, const VkFenceCreateInfo* pCreateInfo);
Move<VkSemaphore>					createSemaphore					(const DeviceInterface& vk, VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo);
Move<VkEvent>						createEvent						(const DeviceInterface& vk, VkDevice device, const VkEventCreateInfo* pCreateInfo);
Move<VkQueryPool>					createQueryPool					(const DeviceInterface& vk, VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo);
Move<VkBuffer>						createBuffer					(const DeviceInterface& vk, VkDevice device, const VkBufferCreateInfo* pCreateInfo);
Move<VkBufferView>					createBufferView				(const DeviceInterface& vk, VkDevice device, const VkBufferViewCreateInfo* pCreateInfo);
Move<VkImage>						createImage						(const DeviceInterface& vk, VkDevice device, const VkImageCreateInfo* pCreateInfo);
Move<VkImageView>					createImageView					(const DeviceInterface& vk, VkDevice device, const VkImageViewCreateInfo* pCreateInfo);
Move<VkAttachmentView>				createAttachmentView			(const DeviceInterface& vk, VkDevice device, const VkAttachmentViewCreateInfo* pCreateInfo);
Move<VkShaderModule>				createShaderModule				(const DeviceInterface& vk, VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo);
Move<VkShader>						createShader					(const DeviceInterface& vk, VkDevice device, const VkShaderCreateInfo* pCreateInfo);
Move<VkPipelineCache>				createPipelineCache				(const DeviceInterface& vk, VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo);
Move<VkPipelineLayout>				createPipelineLayout			(const DeviceInterface& vk, VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo);
Move<VkSampler>						createSampler					(const DeviceInterface& vk, VkDevice device, const VkSamplerCreateInfo* pCreateInfo);
Move<VkDescriptorSetLayout>			createDescriptorSetLayout		(const DeviceInterface& vk, VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo);
Move<VkDescriptorPool>				createDescriptorPool			(const DeviceInterface& vk, VkDevice device, VkDescriptorPoolUsage poolUsage, deUint32 maxSets, const VkDescriptorPoolCreateInfo* pCreateInfo);
Move<VkDynamicViewportState>		createDynamicViewportState		(const DeviceInterface& vk, VkDevice device, const VkDynamicViewportStateCreateInfo* pCreateInfo);
Move<VkDynamicRasterState>			createDynamicRasterState		(const DeviceInterface& vk, VkDevice device, const VkDynamicRasterStateCreateInfo* pCreateInfo);
Move<VkDynamicColorBlendState>		createDynamicColorBlendState	(const DeviceInterface& vk, VkDevice device, const VkDynamicColorBlendStateCreateInfo* pCreateInfo);
Move<VkDynamicDepthStencilState>	createDynamicDepthStencilState	(const DeviceInterface& vk, VkDevice device, const VkDynamicDepthStencilStateCreateInfo* pCreateInfo);
Move<VkFramebuffer>					createFramebuffer				(const DeviceInterface& vk, VkDevice device, const VkFramebufferCreateInfo* pCreateInfo);
Move<VkRenderPass>					createRenderPass				(const DeviceInterface& vk, VkDevice device, const VkRenderPassCreateInfo* pCreateInfo);
Move<VkCmdPool>						createCommandPool				(const DeviceInterface& vk, VkDevice device, const VkCmdPoolCreateInfo* pCreateInfo);
Move<VkCmdBuffer>					createCommandBuffer				(const DeviceInterface& vk, VkDevice device, const VkCmdBufferCreateInfo* pCreateInfo);