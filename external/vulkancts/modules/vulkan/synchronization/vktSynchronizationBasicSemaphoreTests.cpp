/*------------------------------------------------------------------------
 * Vulkan Conformance Tests
 * ------------------------
 *
 * Copyright (c) 2016 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *//*!
 * \file
 * \brief Synchronization semaphore basic tests
 *//*--------------------------------------------------------------------*/

#include "vktSynchronizationBasicSemaphoreTests.hpp"
#include "vktTestCaseUtil.hpp"
#include "vktSynchronizationUtil.hpp"

#include "vkDefs.hpp"
#include "vkPlatform.hpp"
#include "vkQueryUtil.hpp"


#include "vkRef.hpp"

namespace vkt
{
namespace synchronization
{
namespace
{

using namespace vk;

#define FENCE_WAIT	~0ull

tcu::TestStatus basicOneQueueCase (Context& context)
{
	const DeviceInterface&			vk					= context.getDeviceInterface();
	const VkDevice					device				= context.getDevice();
	const VkQueue					queue				= context.getUniversalQueue();
	const deUint32					queueFamilyIndex	= context.getUniversalQueueFamilyIndex();
	const VkSemaphoreCreateInfo		semaphoreInfo		=
														{
															VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,	//VkStructureType			sType;
															DE_NULL,									//const void*				pNext;
															0u											//VkSemaphoreCreateFlags	flags;
														};
	const Unique<VkSemaphore>		semaphore			(createSemaphore (vk, device, &semaphoreInfo, DE_NULL));
	const Unique<VkCommandPool>		cmdPool				(makeCommandPool(vk, device, queueFamilyIndex));
	const Unique<VkCommandBuffer>	cmdBuffer			(makeCommandBuffer(vk, device, *cmdPool));
	const VkCommandBufferBeginInfo	info				=
														{
															VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,	// VkStructureType                          sType;
															DE_NULL,										// const void*                              pNext;
															VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,	// VkCommandBufferUsageFlags                flags;
															DE_NULL,										// const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
														};
	const VkPipelineStageFlags		stageBits[]			= { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
	const VkSubmitInfo				submitInfo[2]		=
														{
															{
																VK_STRUCTURE_TYPE_SUBMIT_INFO,		// VkStructureType			sType;
																DE_NULL,							// const void*				pNext;
																0u,									// deUint32					waitSemaphoreCount;
																DE_NULL,							// const VkSemaphore*		pWaitSemaphores;
																(const VkPipelineStageFlags*)DE_NULL,
																1u,									// deUint32					commandBufferCount;
																&cmdBuffer.get(),					// const VkCommandBuffer*	pCommandBuffers;
																1u,									// deUint32					signalSemaphoreCount;
																&semaphore.get(),					// const VkSemaphore*		pSignalSemaphores;
															},
															{
																VK_STRUCTURE_TYPE_SUBMIT_INFO,		// VkStructureType				sType;
																DE_NULL,							// const void*					pNext;
																1u,									// deUint32						waitSemaphoreCount;
																&semaphore.get(),					// const VkSemaphore*			pWaitSemaphores;
																stageBits,							// const VkPipelineStageFlags*	pWaitDstStageMask;
																1u,									// deUint32						commandBufferCount;
																&cmdBuffer.get(),					// const VkCommandBuffer*		pCommandBuffers;
																0u,									// deUint32						signalSemaphoreCount;
																DE_NULL,							// const VkSemaphore*			pSignalSemaphores;
															}
														};
	const VkFenceCreateInfo			fenceInfo			=
														{
															VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, // VkStructureType		sType;
															DE_NULL,							 // const void*			pNext;
															0u,									 // VkFenceCreateFlags	flags;
														};
	const Unique<VkFence>			fence				(createFence(vk, device, &fenceInfo));

	VK_CHECK(vk.beginCommandBuffer(*cmdBuffer, &info));
	endCommandBuffer(vk, *cmdBuffer);
	VK_CHECK(vk.queueSubmit(queue, 2u, submitInfo, *fence));

	if (VK_SUCCESS != vk.waitForFences(device, 1u, &fence.get(), DE_TRUE, FENCE_WAIT))
		return tcu::TestStatus::fail("Basic semaphore tests with one queue failed");

	return tcu::TestStatus::pass("Basic semaphore tests with one queue passed");
}

tcu::TestStatus basicMultiQueueCase (Context& context)
{
	enum {NO_MATCH_FOUND = ~((deUint32)0)};
	enum QueuesIndexes {FIRST = 0, SECOND, COUNT};

	struct Queues
	{
		VkQueue		queue;
		deUint32	queueFamilyIndex;
	};


	const DeviceInterface&					vk							= context.getDeviceInterface();
	const InstanceInterface&				instance					= context.getInstanceInterface();
	const VkPhysicalDevice					physicalDevice				= context.getPhysicalDevice();
	vk::Move<vk::VkDevice>					logicalDevice;
	std::vector<VkQueueFamilyProperties>	queueFamilyProperties;
	VkDeviceCreateInfo						deviceInfo;
	VkPhysicalDeviceFeatures				deviceFeatures;
	const float								queuePriorities[COUNT]		= {1.0f, 1.0f};
	VkDeviceQueueCreateInfo					queueInfos[COUNT];
	Queues									queues[COUNT]				=
																		{
																			{DE_NULL, NO_MATCH_FOUND},
																			{DE_NULL, NO_MATCH_FOUND}
																		};
	const VkCommandBufferBeginInfo			info						=
																		{
																			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,	// VkStructureType                          sType;
																			DE_NULL,										// const void*                              pNext;
																			VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,	// VkCommandBufferUsageFlags                flags;
																			DE_NULL,										// const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
																		};
	const VkSemaphoreCreateInfo				semaphoreInfo				=
																		{
																			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,	//VkStructureType			sType;
																			DE_NULL,									//const void*				pNext;
																			0u											//VkSemaphoreCreateFlags	flags;
																		};
	Move<VkSemaphore>						semaphore;
	Move<VkCommandPool>						cmdPool[COUNT];
	Move<VkCommandBuffer>					cmdBuffer[COUNT];
	const VkPipelineStageFlags				stageBits[]					= { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
	VkSubmitInfo							submitInfo[COUNT];
	const VkFenceCreateInfo					fenceInfo					=
																		{
																			VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, // VkStructureType		sType;
																			DE_NULL,							 // const void*			pNext;
																			0u,									 // VkFenceCreateFlags	flags;
																		};
	Move<VkFence>							fence[COUNT];

	queueFamilyProperties = getPhysicalDeviceQueueFamilyProperties(instance, physicalDevice);

	for (deUint32 queueNdx = 0; queueNdx < queueFamilyProperties.size(); ++queueNdx)
	{
		if (NO_MATCH_FOUND == queues[FIRST].queueFamilyIndex)
			queues[FIRST].queueFamilyIndex = queueNdx;

		if (queues[FIRST].queueFamilyIndex != queueNdx || queueFamilyProperties[queueNdx].queueCount > 1u)
		{
			queues[SECOND].queueFamilyIndex = queueNdx;
			break;
		}
	}

	if (queues[FIRST].queueFamilyIndex == NO_MATCH_FOUND || queues[SECOND].queueFamilyIndex == NO_MATCH_FOUND)
		TCU_THROW(NotSupportedError, "Queues couldn't be created");

	for (int queueNdx = 0; queueNdx < COUNT; ++queueNdx)
	{
		VkDeviceQueueCreateInfo queueInfo;
		deMemset(&queueInfo, 0, sizeof(queueInfo));

		queueInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.pNext				= DE_NULL;
		queueInfo.flags				= (VkDeviceQueueCreateFlags)0u;
		queueInfo.queueFamilyIndex	= queues[queueNdx].queueFamilyIndex;
		queueInfo.queueCount		= (queues[FIRST].queueFamilyIndex == queues[SECOND].queueFamilyIndex) ? 2 : 1;
		queueInfo.pQueuePriorities	= queuePriorities;

		queueInfos[queueNdx]		= queueInfo;

		if (queues[FIRST].queueFamilyIndex == queues[SECOND].queueFamilyIndex)
			break;
	}

	deMemset(&deviceInfo, 0, sizeof(deviceInfo));
	instance.getPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

	deviceInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext					= DE_NULL;
	deviceInfo.enabledExtensionCount	= 0u;
	deviceInfo.ppEnabledExtensionNames	= DE_NULL;
	deviceInfo.enabledLayerCount		= 0u;
	deviceInfo.ppEnabledLayerNames		= DE_NULL;
	deviceInfo.pEnabledFeatures			= &deviceFeatures;
	deviceInfo.queueCreateInfoCount		= (queues[FIRST].queueFamilyIndex == queues[SECOND].queueFamilyIndex) ? 1 : COUNT;
	deviceInfo.pQueueCreateInfos		= queueInfos;

	logicalDevice = vk::createDevice(instance, physicalDevice, &deviceInfo);

	for (deUint32 queueReqNdx = 0; queueReqNdx < COUNT; ++queueReqNdx)
	{
		if (queues[FIRST].queueFamilyIndex == queues[SECOND].queueFamilyIndex)
			vk.getDeviceQueue(*logicalDevice, queues[queueReqNdx].queueFamilyIndex, queueReqNdx, &queues[queueReqNdx].queue);
		else
			vk.getDeviceQueue(*logicalDevice, queues[queueReqNdx].queueFamilyIndex, 0u, &queues[queueReqNdx].queue);
	}

	semaphore			= (createSemaphore (vk,*logicalDevice, &semaphoreInfo, DE_NULL));
	cmdPool[FIRST]		= (makeCommandPool(vk, *logicalDevice, queues[FIRST].queueFamilyIndex));
	cmdPool[SECOND]		= (makeCommandPool(vk, *logicalDevice, queues[SECOND].queueFamilyIndex));
	cmdBuffer[FIRST]	= (makeCommandBuffer(vk, *logicalDevice, *cmdPool[FIRST]));
	cmdBuffer[SECOND]	= (makeCommandBuffer(vk, *logicalDevice, *cmdPool[SECOND]));

	submitInfo[FIRST].sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo[FIRST].pNext					= DE_NULL;
	submitInfo[FIRST].waitSemaphoreCount	= 0u;
	submitInfo[FIRST].pWaitSemaphores		= DE_NULL;
	submitInfo[FIRST].pWaitDstStageMask		= (const VkPipelineStageFlags*)DE_NULL;
	submitInfo[FIRST].commandBufferCount	= 1u;
	submitInfo[FIRST].pCommandBuffers		= &cmdBuffer[FIRST].get();
	submitInfo[FIRST].signalSemaphoreCount	= 1u;
	submitInfo[FIRST].pSignalSemaphores		= &semaphore.get();

	submitInfo[SECOND].sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo[SECOND].pNext					= DE_NULL;
	submitInfo[SECOND].waitSemaphoreCount		= 1u;
	submitInfo[SECOND].pWaitSemaphores			= &semaphore.get();
	submitInfo[SECOND].pWaitDstStageMask		= stageBits;
	submitInfo[SECOND].commandBufferCount		= 1u;
	submitInfo[SECOND].pCommandBuffers			= &cmdBuffer[SECOND].get();
	submitInfo[SECOND].signalSemaphoreCount		= 0u;
	submitInfo[SECOND].pSignalSemaphores		= DE_NULL;

	VK_CHECK(vk.beginCommandBuffer(*cmdBuffer[FIRST], &info));
	endCommandBuffer(vk, *cmdBuffer[FIRST]);
	VK_CHECK(vk.beginCommandBuffer(*cmdBuffer[SECOND], &info));
	endCommandBuffer(vk, *cmdBuffer[SECOND]);

	fence[FIRST]  = (createFence(vk, *logicalDevice, &fenceInfo));
	fence[SECOND] = (createFence(vk, *logicalDevice, &fenceInfo));

	VK_CHECK(vk.queueSubmit(queues[FIRST].queue, 1u, &submitInfo[FIRST], *fence[FIRST]));
	VK_CHECK(vk.queueSubmit(queues[SECOND].queue, 1u, &submitInfo[SECOND], *fence[SECOND]));

	if (VK_SUCCESS != vk.waitForFences(*logicalDevice, 1u, &fence[FIRST].get(), DE_TRUE, FENCE_WAIT))
		return tcu::TestStatus::fail("Basic semaphore tests with multi queue failed");

	if (VK_SUCCESS != vk.waitForFences(*logicalDevice, 1u, &fence[SECOND].get(), DE_TRUE, FENCE_WAIT))
		return tcu::TestStatus::fail("Basic semaphore tests with multi queue failed");

	{
		VkSubmitInfo swapInfo				= submitInfo[SECOND];
		submitInfo[SECOND]					= submitInfo[FIRST];
		submitInfo[FIRST]					= swapInfo;
		submitInfo[SECOND].pCommandBuffers	= &cmdBuffer[SECOND].get();
		submitInfo[FIRST].pCommandBuffers	= &cmdBuffer[FIRST].get();
	}

	VK_CHECK(vk.resetFences(*logicalDevice, 1u, &fence[FIRST].get()));
	VK_CHECK(vk.resetFences(*logicalDevice, 1u, &fence[SECOND].get()));

	VK_CHECK(vk.queueSubmit(queues[SECOND].queue, 1u, &submitInfo[SECOND], *fence[SECOND]));
	VK_CHECK(vk.queueSubmit(queues[FIRST].queue, 1u, &submitInfo[FIRST], *fence[FIRST]));

	if (VK_SUCCESS != vk.waitForFences(*logicalDevice, 1u, &fence[FIRST].get(), DE_TRUE, FENCE_WAIT))
		return tcu::TestStatus::fail("Basic semaphore tests with multi queue failed");

	if (VK_SUCCESS != vk.waitForFences(*logicalDevice, 1u, &fence[SECOND].get(), DE_TRUE, FENCE_WAIT))
		return tcu::TestStatus::fail("Basic semaphore tests with multi queue failed");

	return tcu::TestStatus::pass("Basic semaphore tests with multi queue passed");
}

} // anonymous

tcu::TestCaseGroup* createBasicSemaphoreTests (tcu::TestContext& testCtx)
{
	de::MovePtr<tcu::TestCaseGroup> basicTests(new tcu::TestCaseGroup(testCtx, "semaphore", "Basic semaphore tests"));
	addFunctionCase(basicTests.get(), "one_queue",   "Basic semaphore tests with one queue",   basicOneQueueCase);
	addFunctionCase(basicTests.get(), "multi_queue", "Basic semaphore tests with multi queue", basicMultiQueueCase);

	return basicTests.release();
}

} // synchronization
} // vkt
