/*
 * Ring Buffer Library for Arduino
 *
 * Copyright Jean-Luc Béchennec 2018
 *
 * This software is distributed under the GNU Public Licence v2 (GPLv2)
 *
 * Please read the LICENCE file
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

 /*
  * Note about interrupt safe implementation
  *
  * To be safe from interrupts, a sequence of C instructions must be framed
  * by a pair of interrupt disable and enable instructions and ensure that the
  * compiler will not move writing of variables to memory outside the protected
  * area. This is called a critical section. Usually the manipulated variables
  * receive the volatile qualifier so that any changes are immediately written
  * to memory. Here the approach is different. First of all you have to know
  * that volatile is useless if the variables are updated in a function and
  * that this function is called within the critical section. Indeed, the
  * semantics of the C language require that the variables in memory be updated
  * before returning from the function. But beware of function inlining because
  * the compiler may decide to delete a function call in favor of simply
  * inserting its code in the caller. To force the compiler to use a real
  * function call, __attribute__((noinline)) have been added to the push and
  * pop functions. In this way the lockedPush and lockedPop functions ensure
  * that in the critical section a push and pop function call respectively will
  * be used by the compiler. This ensures that, because of the function call,
  * the variables are written to memory in the critical section and also
  * ensures that, despite the reorganization of the instructions due to
  * optimizations, the critical section will be well opened and closed at the
  * right place because function calls, due to potential side effects, are not
  * subject to such reorganizations.
  */

#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <Arduino.h>

template <typename rg_element_t, uint8_t __maxSize__>
class RingBuffer
{
private:
  rg_element_t mBuffer[__maxSize__];
  uint8_t mReadIndex;
  uint8_t mSize;

  uint8_t writeIndex();

public:
  /* Constructor. Init mReadIndex to 0 and mSize to 0 */
  RingBuffer();
  /* Push a data at the end of the buffer */
  bool push(const rg_element_t inElement) __attribute__ ((noinline));
  /* Push a data at the end of the buffer. Copy it from its pointer */
  bool push(const rg_element_t * const inElement) __attribute__ ((noinline));
  bool push(const rg_element_t* inElements, uint8_t length) __attribute__((noinline));
  /* Push a data at the end of the buffer with interrupts disabled */
  bool lockedPush(const rg_element_t inElement);
  /* Push a data at the end of the buffer with interrupts disabled. Copy it from its pointer */
  bool lockedPush(const rg_element_t * const inElement);
  /* Pop the data at the beginning of the buffer */
  bool pop(rg_element_t &outElement) __attribute__ ((noinline));
  /* Pop the data at the beginning of the buffer */
  bool pop() __attribute__((noinline));
  /* Pop the data at the beginning of the buffer with interrupt disabled */
  bool lockedPop(rg_element_t &outElement);
  /* Return true if the buffer is full */
  bool isFull()  { return mSize == __maxSize__; }
  /* Return true if the buffer is empty */
  bool isEmpty() { return mSize == 0; }
  /* Reset the buffer  to an empty state */
  void clear()   { mSize = 0; }
  /* return the size of the buffer */
  uint8_t size() { return mSize; }
  /* return the maximum size of the buffer */
  uint8_t maxSize() { return __maxSize__; }
  /* access the buffer using array syntax, not interrupt safe */
  rg_element_t &operator[](uint8_t inIndex);
};

template <typename rg_element_t, uint8_t __maxSize__>
uint8_t RingBuffer<rg_element_t, __maxSize__>::writeIndex()
{
 uint16_t wi = (uint16_t)mReadIndex + (uint16_t)mSize;
 if (wi >= (uint16_t)__maxSize__) wi -= (uint16_t)__maxSize__;
 return (uint8_t)wi;
}

template <typename rg_element_t, uint8_t __maxSize__>
RingBuffer<rg_element_t, __maxSize__>::RingBuffer() :
mReadIndex(0),
mSize(0)
{
}

template <typename rg_element_t, uint8_t __maxSize__>
bool RingBuffer<rg_element_t, __maxSize__>::push(const rg_element_t inElement)
{
  if (isFull()) return false;
  mBuffer[writeIndex()] = inElement;
  mSize++;
  return true;
}

template <typename rg_element_t, uint8_t __maxSize__>
bool RingBuffer<rg_element_t, __maxSize__>::push(const rg_element_t* inElements, uint8_t length)
{
	if (isFull()) return false;
	memcpy(mBuffer + writeIndex(), inElements, length);
	mSize+=length;
	return true;
}

template <typename rg_element_t, uint8_t __maxSize__>
bool RingBuffer<rg_element_t, __maxSize__>::push(const rg_element_t * const inElement)
{
  if (isFull()) return false;
  mBuffer[writeIndex()] = *inElement;
  mSize++;
  return true;
}

template <typename rg_element_t, uint8_t __maxSize__>
bool RingBuffer<rg_element_t, __maxSize__>::lockedPush(const rg_element_t inElement)
{
  noInterrupts();
  bool result = push(inElement);
  interrupts();
  return result;
}

template <typename rg_element_t, uint8_t __maxSize__>
bool RingBuffer<rg_element_t, __maxSize__>::lockedPush(const rg_element_t * const inElement)
{
  noInterrupts();
  bool result = push(inElement);
  interrupts();
  return result;
}

template <typename rg_element_t, uint8_t __maxSize__>
bool RingBuffer<rg_element_t, __maxSize__>::pop(rg_element_t &outElement)
{
  if (isEmpty()) return false;
  outElement = mBuffer[mReadIndex];
  mReadIndex++;
  mSize--;
  if (mReadIndex == __maxSize__) mReadIndex = 0;
  return true;
}

template <typename rg_element_t, uint8_t __maxSize__>
bool RingBuffer<rg_element_t, __maxSize__>::pop()
{
	if (isEmpty()) return false;
	mReadIndex++;
	mSize--;
	if (mReadIndex == __maxSize__) mReadIndex = 0;
	return true;
}

template <typename rg_element_t, uint8_t __maxSize__>
bool RingBuffer<rg_element_t, __maxSize__>::lockedPop(rg_element_t &outElement)
{
  noInterrupts();
  bool result = pop(outElement);
  interrupts();
  return result;
}

template <typename rg_element_t, uint8_t __maxSize__>
rg_element_t &RingBuffer<rg_element_t, __maxSize__>::operator[](uint8_t inIndex)
{
  if (inIndex >= mSize) return mBuffer[0];
  uint16_t index = (uint16_t)mReadIndex + (uint16_t)inIndex;
  if (index >= (uint16_t)__maxSize__) index -= (uint16_t)__maxSize__;
  return mBuffer[(uint8_t)index];
}

#endif /* __RINGBUFFER_H__ */