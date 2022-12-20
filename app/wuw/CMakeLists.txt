# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.cppom/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

# cmake_minimum_required(VERSION 3.17)
cmake_minimum_required(VERSION 3.10)

# Declares and names the project.

project("wakeup")

# Kaldi based setup
# set(USE_KALDI TRUE)
# set(USE_CIPHER TRUE)
set(USE_KALDI FALSE)
set(USE_CIPHER FALSE)

# Root dir of libraries
if(COMPILE_PC)
	get_filename_component(LIBS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../sdk-build/libs" ABSOLUTE)
	set(LIB_ROOT "${LIBS_PATH}")
	message("LIBROOT : ${LIB_ROOT}")

	# Add CUDA support
	find_package(CUDAToolkit)
else()
	get_filename_component(LIBS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../android/libs" ABSOLUTE)
	# set(LIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../android/libs") # Note about soft link
	set(LIB_ROOT "${LIBS_PATH}")
endif()


if(USE_KALDI)
	# Add macro
	add_definitions(-DUSE_KALDI)

	if(COMPILE_PC)
		set(LIB_KALDI "${LIB_ROOT}/kaldi")
		set(LIB_BOTAN "${LIB_ROOT}/botan")

		set(KALDI_ROOT "${LIB_KALDI}/kaldi")
		set(OTHER_ROOT "${LIB_KALDI}/other")

		set(LIB_KALDI_LIB 			"${KALDI_ROOT}/lib")
		set(LIB_KALDI_HEADER 		"${KALDI_ROOT}/include")

		set(LIB_FST_LIB 			"${OTHER_ROOT}/lib")
		set(LIB_FST_HEADER 			"${OTHER_ROOT}/include")

	else()
		# Add lib
		if ("x${ANDROID_ABI}" STREQUAL "xarmeabi-v7a")
			set(OPENBLAS_ARCH "armv7")
			set(KALDI_SUFFIX "arm_32")
		elseif ("x${ANDROID_ABI}" STREQUAL "xarm64-v8a")
			set(OPENBLAS_ARCH "armv8")
			set(KALDI_SUFFIX "arm_64")
		elseif ("x${ANDROID_ABI}" STREQUAL "xx86")
			set(OPENBLAS_ARCH "atom")
			set(KALDI_SUFFIX "x86")
		else ("x${ANDROID_ABI}" STREQUAL "xarmeabi-v7a")
			set(OPENBLAS_ARCH "atom")
			set(KALDI_SUFFIX "x86_64")
		endif ("x${ANDROID_ABI}" STREQUAL "xarmeabi-v7a")

		set(LIB_KALDI "${LIB_ROOT}/kaldi")
		set(LIB_BOTAN "${LIB_ROOT}/botan")
		set(KALDI_ROOT "${LIB_KALDI}/kaldi_${KALDI_SUFFIX}/kaldi")
		set(OTHER_ROOT "${LIB_KALDI}/kaldi_${KALDI_SUFFIX}/local")

		set(LIB_KALDI_LIB 			"${KALDI_ROOT}/src")
		set(LIB_KALDI_HEADER 		"${KALDI_ROOT}/src")

		set(LIB_FST_LIB 			"${OTHER_ROOT}/lib")
		set(LIB_FST_HEADER 			"${OTHER_ROOT}/include")

	endif()
endif()

if(USE_CIPHER)
	# Add macro
	add_definitions(-DUSE_CIPHER)
endif()

set(LIB_TENSORFLOW "${LIB_ROOT}/tensorlite")
set(LIB_AUBIO "${LIB_ROOT}/aubio")

set(LIB_TENSORFLOW_HEADER 	"${LIB_TENSORFLOW}/include")
set(LIB_AUBIO_HEADER 		"${LIB_AUBIO}/include")

if(COMPILE_PC)
	# pc compile
	set(LIB_TENSORFLOW_LIB 		"${LIB_TENSORFLOW}/lib")
	set(LIB_AUBIO_LIB 			"${LIB_AUBIO}/lib")
	set(LIB_BOTAN_LIB 			"${LIB_BOTAN}/lib")

	set(LIB_BOTAN_HEADER 		"${LIB_BOTAN}/include")
else()
	# android compile
	set(LIB_TENSORFLOW_LIB 		"${LIB_TENSORFLOW}/lib/${ANDROID_ABI}")
	set(LIB_AUBIO_LIB 			"${LIB_AUBIO}/lib/${ANDROID_ABI}")
	set(LIB_BOTAN_LIB 			"${LIB_BOTAN}/${ANDROID_ABI}/lib")

	set(LIB_BOTAN_HEADER 		"${LIB_BOTAN}/${ANDROID_ABI}/include")
endif()

if(NOT COMPILE_PC)
	find_library( # Sets the name of the path variable.
			log-lib

			# Specifies the name of the NDK library that
			# you want CMake to locate.
			log
			)
endif()

# All source
add_library( # Sets the name of the library.
		wakeup

		# Sets the library as a shared library.
		SHARED
		native-lib.cpp
		common/feature_extractor.cpp
		common/utils.cpp
		common/cipher.cpp
		common/config_reader.cpp
		main_process.cpp
		e2e/siri_wuw.cpp
		e2e/cra_wuw.cpp
		kaldi/kaldi_based_wuw.cpp
		kaldi/model.cc
		kaldi/classifier.cc
		kaldi/decodable_online_looped_bdi.cc
		kaldi/lattice_analysis.cc
		kaldi/lattice_faster_decoder_vbdi.cc
		kaldi/lattice_faster_online_decoder_bdi.cc
		kaldi/online_endpoint_bdi.cc
		kaldi/online_ivector_feature_bdi.cc
		kaldi/online_nnet3_decoding_bdi.cc)

# include dir
target_include_directories(
		wakeup

		PRIVATE

		${LIB_TENSORFLOW_HEADER}
		${LIB_AUBIO_HEADER}
)

# libraries dir
if(COMPILE_PC)
#	target_link_libraries(
#			wakeup
#			CUDA::cudart
#	)

	target_link_libraries( # Specifies the target library.
			wakeup

			# Links the target library to the log library
			# included in the NDK.
			${log-lib}
			${LIB_TENSORFLOW_LIB}/libtensorflowlite_c.so

			${LIB_AUBIO_LIB}/libaubio.a
			)
else()
	target_link_libraries( # Specifies the target library.
			wakeup

			# Links the target library to the log library
			# included in the NDK.
			${log-lib}
			${LIB_TENSORFLOW_LIB}/libtensorflowlite_jni.so
			${LIB_AUBIO_LIB}/libaubio.a
			)
endif()


if(USE_KALDI)
	# include dir
	target_include_directories(
			wakeup

			PRIVATE

			${LIB_KALDI_HEADER}
			${LIB_FST_HEADER}
	)

	target_link_libraries( # Specifies the target library.
			wakeup

			${LIB_KALDI_LIB}/online2/kaldi-online2.a
			${LIB_KALDI_LIB}/decoder/kaldi-decoder.a
			${LIB_KALDI_LIB}/ivector/kaldi-ivector.a
			${LIB_KALDI_LIB}/gmm/kaldi-gmm.a
			${LIB_KALDI_LIB}/nnet3/kaldi-nnet3.a
			${LIB_KALDI_LIB}/tree/kaldi-tree.a
			${LIB_KALDI_LIB}/feat/kaldi-feat.a
			${LIB_KALDI_LIB}/lat/kaldi-lat.a
			${LIB_KALDI_LIB}/lm/kaldi-lm.a
			${LIB_KALDI_LIB}/hmm/kaldi-hmm.a
			${LIB_KALDI_LIB}/transform/kaldi-transform.a
			${LIB_KALDI_LIB}/cudamatrix/kaldi-cudamatrix.a
			${LIB_KALDI_LIB}/matrix/kaldi-matrix.a
			${LIB_KALDI_LIB}/fstext/kaldi-fstext.a
			${LIB_KALDI_LIB}/util/kaldi-util.a
			${LIB_KALDI_LIB}/base/kaldi-base.a
	)

	# libraries dir
	if(COMPILE_PC)
		target_link_libraries( # Specifies the target library.
				wakeup

				${LIB_KALDI_LIB}/cudadecoder/kaldi-cudadecoder.a
				${LIB_KALDI_LIB}/cudafeat/kaldi-cudafeat.a

				${LIB_FST_LIB}/libfst.a
				${LIB_FST_LIB}/libfstfar.a
				${LIB_FST_LIB}/libfstfarscript.a
				${LIB_FST_LIB}/libfstlookahead.a
				${LIB_FST_LIB}/libfstngram.a
				${LIB_FST_LIB}/libfstscript.a
				)
	else()
		target_link_libraries( # Specifies the target library.
				wakeup

				${LIB_FST_LIB}/libfst.a
				${LIB_FST_LIB}/libfstngram.a
				${LIB_FST_LIB}/libopenblas_${OPENBLAS_ARCH}-r0.3.7.a
				${LIB_FST_LIB}/libclapack.a
				${LIB_FST_LIB}/liblapack.a
				${LIB_FST_LIB}/libblas.a
				${LIB_FST_LIB}/libf2c.a
				)
	endif()

endif()

if (USE_CIPHER)
	# include dir
	target_include_directories(
			wakeup

			PRIVATE
			${LIB_BOTAN_HEADER}
	)

	# libraries dir
	if(COMPILE_PC)
		target_link_libraries( # Specifies the target library.
				wakeup

				${LIB_BOTAN_LIB}/libbotan-3.a
				)
	else()
		target_link_libraries( # Specifies the target library.
				wakeup

				${LIB_BOTAN_LIB}/libbotan-2.so
				)
	endif()
endif()
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.


