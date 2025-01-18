/**
 * @file demo.c
 * @brief Demo application for the MZHV driver.
 */


#include <assert.h>
#include <intrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>


 /**************************************************************************************************
 * Defines
 **************************************************************************************************/
#define DRIVER_MAP       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1337, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define DRIVER_UNMAP     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x2137, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define DEVICE_NAME      L"\\\\.\\MZHV"
#define PAGE_SIZE        4096


 /**************************************************************************************************
 * Local function declarations
 **************************************************************************************************/
static BOOL changeMapping(VOID* originalAddress, VOID* rwAddress, VOID* fetchAddress);


static BOOL removeMappingChange(VOID* originalAddress);


static VOID printBytes(VOID* address, SIZE_T noOfBytes);


__declspec(noinline) static VOID* getAddress(VOID);


static ULONG_PTR getEntryPointAddress(VOID* function);


/**************************************************************************************************
* Global function definitions
**************************************************************************************************/
/**
 * @brief Returns a random number between 10 and 20.
 * 
 * @note This function is not static to make the example more interesting.
 *       If the function was static, it's address would equal to &getRandomNumber.
 *       If the function is not static, the address from &getRandomNumber points
 *       to the jump instruction, which jumps to the actual function.
 * 
 * @return INT32 Random number between 10 and 20.
 */
__declspec(noinline) INT32 getRandomNumber(VOID)
{
  INT32 lowerLimit = 10;
  INT32 upperLimit = 20;

  return (rand() % (upperLimit - lowerLimit + 1) + lowerLimit);
}


/**
 * @brief Demo for swapping pages.
 */
VOID swapPageDemo(VOID)
{
  VOID* firstPage = VirtualAlloc(NULL, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  VOID* secondPage = VirtualAlloc(NULL, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

  assert(firstPage != NULL);
  assert(secondPage != NULL);

  memset(firstPage, 'A', PAGE_SIZE);
  memset(secondPage, 'B', PAGE_SIZE);

  printf("Initial state:\n");
  printBytes(firstPage, 10);
  printBytes(secondPage, 10);
  printf("\n");

  BOOL isSuccessful = changeMapping(firstPage, secondPage, firstPage);
  printf("Change mapping: %s\n", isSuccessful ? "SUCCESS" : "FAILURE");
  printBytes(firstPage, 10);
  printBytes(secondPage, 10);
  printf("\n");

  printf("Memset first page to 'C'\n");
  memset(firstPage, 'C', PAGE_SIZE);
  printBytes(firstPage, 10);
  printBytes(secondPage, 10);
  printf("\n");

  isSuccessful = removeMappingChange(firstPage);
  printf("Remove mapping change: %s\n", isSuccessful ? "SUCCESS" : "FAILURE");
  printBytes(firstPage, 10);
  printBytes(secondPage, 10);
  printf("\n");

  VirtualFree(firstPage, 0, MEM_RELEASE);
  VirtualFree(secondPage, 0, MEM_RELEASE);
}


/**
 * @brief Demo for hiding code, requires debugger to be attached.
 */
VOID hideCodeDemo(VOID)
{
  VOID* page = VirtualAlloc(NULL, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  assert(page != NULL);
  memset(page, 0xFF, PAGE_SIZE);

  VOID* codeAddress = getAddress();
  VOID* alignedCodeAddress = (VOID*)(((ULONG_PTR)codeAddress) & ~0xFFFLLU);

  printf("Initial state\n");
  __debugbreak();

  BOOL isSuccessful = changeMapping(alignedCodeAddress, page, alignedCodeAddress);
  printf("Change mapping: %s\n", isSuccessful ? "SUCCESS" : "FAILURE");
  __debugbreak();

  printf("Code executes normally\n");
  __debugbreak();

  isSuccessful = removeMappingChange(alignedCodeAddress);
  printf("Remove mapping change: %s\n", isSuccessful ? "SUCCESS" : "FAILURE");
  __debugbreak();

  VirtualFree(page, 0, MEM_RELEASE);
}


/**
 * @brief Demo for function patching.
 */
VOID functionPatchingDemo(VOID)
{
  srand(time(NULL));

  VOID* patchedPage = VirtualAlloc(NULL, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  VOID* referencePage = VirtualAlloc(NULL, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

  assert(patchedPage != NULL);
  assert(referencePage != NULL);

  // If getRandomNumber is static, then functionEntryPointAddress == &getRandomNumber
  // If that's the case, then replace the instruction below with:
  // ULONG_PTR functionEntryPointAddress = (ULONG_PTR)&getRandomNumber;

  ULONG_PTR functionEntryPointAddress = getEntryPointAddress(&getRandomNumber);
  ULONG_PTR functionPageAddress = functionEntryPointAddress & ~0xFFFLLU;
  VOID* functionPage = (VOID*)functionPageAddress;

  memcpy(patchedPage, functionPage, PAGE_SIZE);
  memcpy(referencePage, functionPage, PAGE_SIZE);

  ULONG_PTR codeOffset = functionEntryPointAddress - functionPageAddress;

  const CHAR* patch = "\x48\xC7\xC0\xFF\xFF\xFF\xFF\xC3";
  memcpy((VOID*)((ULONG_PTR)patchedPage + codeOffset), patch, 8);

  printf("Initial state\n");
  printf("getRandomNumber: %d\n", getRandomNumber());
  printf("functionPage == patchedPage: %d\n", memcmp(functionPage, patchedPage, PAGE_SIZE) == 0);
  printf("functionPage == referencePage: %d\n", memcmp(functionPage, referencePage, PAGE_SIZE) == 0);
  printf("\n");

  BOOL isSuccessful = changeMapping(functionPage, referencePage, patchedPage);
  printf("Change mapping: %s\n", isSuccessful ? "SUCCESS" : "FAILURE");
  printf("getRandomNumber: %d\n", getRandomNumber());
  printf("functionPage == patchedPage: %d\n", memcmp(functionPage, patchedPage, PAGE_SIZE) == 0);
  printf("functionPage == referencePage: %d\n", memcmp(functionPage, referencePage, PAGE_SIZE) == 0);
  printf("\n");

  isSuccessful = removeMappingChange(functionPage);
  printf("Remove mapping change: %s\n", isSuccessful ? "SUCCESS" : "FAILURE");
  printf("getRandomNumber: %d\n", getRandomNumber());
  printf("functionPage == patchedPage: %d\n", memcmp(functionPage, patchedPage, PAGE_SIZE) == 0);
  printf("functionPage == referencePage: %d\n", memcmp(functionPage, referencePage, PAGE_SIZE) == 0);
  printf("\n");

  VirtualFree(patchedPage, 0, MEM_RELEASE);
  VirtualFree(referencePage, 0, MEM_RELEASE);
}


int main(void)
{
  swapPageDemo();
  //hideCodeDemo();
  functionPatchingDemo();
}


/**************************************************************************************************
* Local function definitions
**************************************************************************************************/
/**
 * @brief Changes the mapping of a page.
 * 
 * Calls the driver to change the mapping of a page.
 * 
 * @param originalAddress Page to be changed.
 * @param rwAddress Page to be mapped for read/write access.
 * @param fetchAddress Page to be mapped for code execution.
 * 
 * @return TRUE if the mapping was changed successfully, FALSE otherwise.
 */
static BOOL changeMapping(VOID* originalAddress, VOID* rwAddress, VOID* fetchAddress)
{
  HANDLE deviceHandle = CreateFile(
    DEVICE_NAME,
    GENERIC_READ | GENERIC_WRITE,
    0,
    NULL,
    CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
    NULL);

  if (deviceHandle == INVALID_HANDLE_VALUE)
  {
    return FALSE;
  }

  VOID* addresses[3] = { originalAddress, rwAddress, fetchAddress };
  BOOL isSuccessful = DeviceIoControl(
    deviceHandle,
    DRIVER_MAP,
    &addresses,
    sizeof(addresses),
    NULL,
    0,
    NULL,
    NULL);

  CloseHandle(deviceHandle);
  return isSuccessful;
}


/**
 * @brief Removes a mapping change.
 * 
 * Calls the driver to remove a mapping change.
 * 
 * @param originalAddress Page to be restored.
 * 
 * @return TRUE if the mapping change was restored to default, FALSE otherwise.
 */
static BOOL removeMappingChange(VOID* originalAddress)
{
  HANDLE deviceHandle = CreateFile(
    DEVICE_NAME,
    GENERIC_READ | GENERIC_WRITE,
    0,
    NULL,
    CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
    NULL);

  if (deviceHandle == INVALID_HANDLE_VALUE)
  {
    return FALSE;
  }

  VOID* addresses[1] = { originalAddress };
  BOOL isSuccessful = DeviceIoControl(
    deviceHandle,
    DRIVER_UNMAP,
    &addresses,
    sizeof(addresses),
    NULL,
    0,
    NULL,
    NULL);

  CloseHandle(deviceHandle);
  return isSuccessful;
}


/**
 * @brief Prints the first 10 bytes of a page.
 * 
 * @param address Address of the page.
 * @param noOfBytes Number of bytes to print.
 *
 * @return VOID
 */
static VOID printBytes(VOID* address, SIZE_T noOfBytes)
{
  for (SIZE_T i = 0; i < noOfBytes; i++)
  {
    printf("%02X ", ((BYTE*)address)[i]);
  }

  printf("\n");
}


/**
 * @brief Returns the address of the caller.
 * 
 * @return VOID* Address of the caller.
 */
__declspec(noinline) static VOID* getAddress(VOID)
{
  return _ReturnAddress();
}


/**
 * @brief Returns the entry point address of a function.
 * 
 * Should be used to get actual address of first instruction of a function if the
 * function is not static.
 * 
 * @param function Function
 * 
 * @return addres of entry point
 */
static ULONG_PTR getEntryPointAddress(VOID* function)
{
  ULONG_PTR jmpEntry = (ULONG_PTR)function;
  ULONG_PTR jmpArgument = jmpEntry + 1;

  UINT32 jmpOffset;
  memcpy(&jmpOffset, (VOID*)jmpArgument, sizeof(jmpOffset));

  UINT32 jmpLength = 5;
  ULONG_PTR functionEntryPoint = jmpEntry + jmpLength + jmpOffset;

  return functionEntryPoint;
}
