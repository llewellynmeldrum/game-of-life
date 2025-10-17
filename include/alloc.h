#ifndef ALLOC_H
#define ALLOC_H

#define ALLOC_LOG(n_elements, element_size)({\
	log("ALLOC: %zux%zuB (%zuB)...\n",		\
     	(size_t)n_elements,			\
	(size_t)element_size,			\
	(size_t)(n_elements*element_size));	\
	\
	void* ptr = calloc(n_elements, element_size);			\
	if(!ptr)							\
	logfatalerrno("Failed to allocate %zu * %zu (%zu) bytes.",	\
	       (size_t)n_elements,					\
	       (size_t)element_size,					\
	       (size_t)(n_elements*element_size)			\
	);\
	ptr;\
})

#define ALLOC(n_elements, element_size)({\
	void* ptr = calloc(n_elements, element_size);			\
	if(!ptr)							\
	logfatalerrno("Failed to allocate %zu * %zu (%zu) bytes.",	\
	       (size_t)n_elements,					\
	       (size_t)element_size,					\
	       (size_t)(n_elements*element_size)			\
	);\
	ptr;\
})
#endif // ALLOC_H
