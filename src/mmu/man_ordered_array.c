
// https://web.archive.org/web/20160326122206/http://jamesmolloy.co.uk/tutorial_html/7.-The%20Heap.html
// ordered_array.c -- Implementation for creating, inserting and deleting
// from ordered arrays.
// Written for JamesM's kernel development tutorials.

#include "man_ordered_array.h"


// The below function will return 1 if a < b, else 0.
int8_t standard_lessthan_predicate(type_t a, type_t b)
{
   return (a<b)?1:0;
}

// Create an ordered array.
ordered_array_t create_ordered_array(uint64_t max_size, lessthan_predicate_t less_than)
{
   ordered_array_t to_ret;
   to_ret.array = (void*) kmalloc(max_size*sizeof(type_t));
   memset(to_ret.array, 0, max_size*sizeof(type_t));
   to_ret.size = 0;
   to_ret.max_size = max_size;
   to_ret.less_than = less_than; // assign the function pointer to the less_than function.
   return to_ret;
}

// Place the ordered array at a certain address.
ordered_array_t place_ordered_array(void *addr, uint64_t max_size, lessthan_predicate_t less_than)
{
   ordered_array_t to_ret;
   to_ret.array = (type_t*) addr;
   memset(to_ret.array, 0, max_size*sizeof(type_t));
   to_ret.size = 0;
   to_ret.max_size = max_size;
   to_ret.less_than = less_than;
   return to_ret;
}

// Destroy an ordered array.
void destroy_ordered_array(ordered_array_t *array)
{
// kfree(array->array);
}

// Add an item into the array.
void insert_ordered_array(type_t item, ordered_array_t *array)
{
   assert(array->less_than); // check if array->less_than pointer is not NULL

   // Finding the index to insert the item at.
   uint64_t iterator = 0;
   while ((iterator < array->size) && (array->less_than( array->array[iterator], item ) ) ){ // array->less_than( array->array[iterator], item ) ensures that the array is sorted and no smaller item will entered.
      iterator++;
      
      if (iterator == array->size) // just add at the end of the array.
         array->array[array->size++] = item;
      else // insert the item at the correct index in the middle of the array either right or left.
      {
         type_t tmp = array->array[iterator]; // This item will be shifted to the right.
         array->array[iterator] = item;     // This item will take the place of the item at index iterator.
         while (iterator < array->size) // shift the elements to the right.
         {
            iterator++;
            type_t tmp2 = array->array[iterator];
            array->array[iterator] = tmp;
            tmp = tmp2;
         }
         array->size++;
      }
   }
}

// Lookup the item at index i.
type_t lookup_ordered_array(uint64_t i, ordered_array_t *array)
{
   assert(i < array->size);
   return array->array[i];
}

// Deletes the item at location i from the array.
void remove_ordered_array(uint64_t i, ordered_array_t *array)
{
   while (i < array->size)
   {
       array->array[i] = array->array[i+1];
       i++;
   }
   array->size--;
}



