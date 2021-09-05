.SUFFIXES:

CC 			:= gcc

LIBS 		:= -static-libgcc \
				$(shell pkg-config --libs sdl2) \
				$(shell pkg-config --libs glew) \
				$(shell pkg-config --libs gl) \
				-Wl,-Bstatic -lm

# CFLAGS  	:= -Wall -Wextra -g -pg -O2 -DDEBUG
CFLAGS  	:= -Wall -Wextra -g -pg -DDEBUG
# LFLAGS 		:= -g -pg -O2 -lc
LFLAGS 		:= -g -pg -lc
MAKE 		:= make
SOURCES 	:= src
BUILD		:= build
IMAGES		:= images
SOUNDS		:= sounds
SHADERS 	:= shaders
MODELS  	:= models
TARGET 		:= final
RESOURCES   := resources

SHADERS_SOURCE_FILE := $(CURDIR)/$(SOURCES)/shader_files.c
SHADERS_HEADER 		:= $(CURDIR)/$(SOURCES)/shader_files.h
SOUNDS_HEADER 		:= $(CURDIR)/$(SOURCES)/sounds.h 
IMAGES_HEADER 		:= $(CURDIR)/$(SOURCES)/images.h 
MODELS_HEADER 		:= $(CURDIR)/$(SOURCES)/models.h 

ifneq ($(BUILD),$(notdir $(CURDIR)))

export TOPDIR	:=	$(CURDIR)

export CFILES  			:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export SOUNDFILES  		:=	$(foreach dir,$(SOUNDS),$(notdir $(wildcard $(dir)/*.wav)))
export V_SHADERFILES 	:=	$(foreach dir,$(SHADERS),$(wildcard $(dir)/*.vs))
export F_SHADERFILES 	:=	$(foreach dir,$(SHADERS),$(wildcard $(dir)/*.fs))
export IMAGEFILES 		:=	$(foreach dir,$(IMAGES),$(notdir $(wildcard $(dir)/*.png)))
export MODELFILES 		:=	$(foreach dir,$(MODELS),$(notdir $(wildcard $(dir)/*.yuk2)))
export ANIMFILES 		:=	$(foreach dir,$(MODELS),$(notdir $(wildcard $(dir)/*.anm)))

export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
				$(foreach dir,$(IMAGES),$(CURDIR)/$(dir)) \
				$(foreach dir,$(MODELS),$(CURDIR)/$(dir)) \
				$(foreach dir,$(SOUNDS),$(CURDIR)/$(dir)) \
				$(foreach dir,$(SHADERS),$(CURDIR)/$(dir))

export OUTPUT := $(CURDIR)/$(TARGET)

export OFILES := $(CFILES:.c=.o)

%.snd: %.wav
	@echo $(notdir $<)
	@(wav2snd/wav2snd $< $(TOPDIR)/$(RESOURCES)/$@)

%.img: %.png
	@echo $(notdir $<)
	@(png2img/png2img $< $(TOPDIR)/$(RESOURCES)/$@)

define append-images-header
	echo "#define IMAGE_"`echo $(basename $1) | tr . _ | tr ' ' _ | tr [a-z] [A-Z]` "\"$(RESOURCES)/$1\"" >> $(IMAGES_HEADER);
endef

define append-sounds-header
	echo "#define SOUND_"`echo $(basename $1) | tr . _ | tr ' ' _ | tr [a-z] [A-Z]` "\"$(RESOURCES)/$1\"" >> $(SOUNDS_HEADER);
endef

define append-models-header
	echo "#define MODEL_"`echo $(basename $1) | tr . _ | tr ' ' _ | tr [a-z] [A-Z]` "\"$(MODELS)/$1\"" >> $(MODELS_HEADER);
endef

define append-models-header-anim
	echo "#define ANIMATION_"`echo $(basename $1) | tr . _ | tr ' ' _ | tr [a-z] [A-Z]` "\"$(MODELS)/$1\"" >> $(MODELS_HEADER);
endef

define append-shaders-header-vs
	
	awk -F, 'match($$0, /(^uniform [^ \t]*)[ \t]*([a-zA-Z]*)/, result) { \
	    print "#ifndef VUNIFORMS_" toupper(result[2]) "_DEF"; \
	    print "#define VUNIFORMS_" toupper(result[2]) "_DEF"; \
	    print "\tVUNIFORM_" result[2] ","; \
	    print "#endif"; \
	}' $1 >> $(SHADERS_HEADER);

endef

define append-shaders-header-fs
	
	awk -F, 'match($$0, /(^uniform [^ \t]*)[ \t]*([a-zA-Z]*)/, result) { \
	    print "#ifndef FUNIFORMS_" toupper(result[2]) "_DEF"; \
	    print "#define FUNIFORMS_" toupper(result[2]) "_DEF"; \
	    print "\tFUNIFORM_" result[2] ","; \
	    print "#endif"; \
	}' $1 >> $(SHADERS_HEADER);

endef

define append-shaders-header-programs
	
	$(eval SHADER_NAME := `echo $(basename $(notdir $1)) | tr . _ | tr ' ' _`)

	echo -e "\tPROGRAM_$(SHADER_NAME)," >> $(SHADERS_HEADER);

endef

define write-shader-c-static-shader
	$(eval SHADER_NAME := `echo $(basename $(notdir $1)) | tr . _ | tr ' ' _`)
	echo -e "static Shader_"$(SHADER_NAME) "shader_$(SHADER_NAME);" >> $(SHADERS_SOURCE_FILE);
endef

define write-shader-c-create
	$(eval SHADER_NAME := `echo $(basename $(notdir $1)) | tr . _ | tr ' ' _`)

	$(eval SHADER := $(basename $(notdir $1)))

	echo -e "\tShader_CompileShader(&shaders[PROGRAM_$(SHADER_NAME)], \"$(SHADERS)/$(SHADER).vs\", \
		\"$(SHADERS)/$(SHADER).fs\");" >> $(SHADERS_SOURCE_FILE);

	awk -F, 'match($$0, /(^uniform [^ \t]*)[ \t]*([a-zA-Z]*)/, result) { \
		print "\tshaders[PROGRAM_'$(SHADER_NAME)'].vUniforms[VUNIFORM_"result[2]"] = glGetUniformLocation(shaders[PROGRAM_" \
		 "'$(SHADER_NAME)'].program, \"" result[2] "\");" \
	}' $(basename $1).vs >> $(SHADERS_SOURCE_FILE);

	awk -F, 'match($$0, /(^uniform [^ \t]*)[ \t]*([a-zA-Z]*)/, result) { \
		print "\tshaders[PROGRAM_'$(SHADER_NAME)'].fUniforms[FUNIFORM_"result[2]"] = glGetUniformLocation(shaders[PROGRAM_" \
		 "'$(SHADER_NAME)'].program, \"" result[2] "\");" \
	}' $(basename $1).fs >> $(SHADERS_SOURCE_FILE);

endef

define write-shader-c-destroy

	$(eval SHADER_NAME := `echo $(basename $(notdir $1)) | tr . _ | tr ' ' _`)

	echo -e "\tShader_DestroyShader(&shaders[PROGRAM_$(SHADER_NAME)]);" >> $(SHADERS_SOURCE_FILE);

endef

define write-shader-c-funcs

	$(eval SHADER := $(basename $(notdir $1)))
	$(eval SHADER_NAME := `echo $(SHADER) | tr . _ | tr ' ' _`)

	echo -e "\nstatic void Shader_Create_"$(SHADER_NAME)"(Shader_$(SHADER_NAME) *shader){" >> $(SHADERS_SOURCE_FILE);

	echo -e "\tShader_CompileShader(&shader->program, &shader->vShader, &shader->fShader, \"$(SHADERS)/$(SHADER).vs\", \
		\"$(SHADERS)/$(SHADER).fs\");" \ >> $(SHADERS_SOURCE_FILE);


	echo -e "}" >> $(SHADERS_SOURCE_FILE);

	echo -e "\nstatic void Shader_Destroy_"$(SHADER_NAME)"(Shader_$(SHADER_NAME) *shader){" >> $(SHADERS_SOURCE_FILE);

	echo -e "\tShader_DestroyShader(shader->program, shader->vShader, shader->fShader);" \
		>> $(SHADERS_SOURCE_FILE);

	echo -e "}" >> $(SHADERS_SOURCE_FILE);

	echo -e "\n"Shader_"$(SHADER_NAME) Shader_GetShader_"$(SHADER_NAME)"(void){" >> $(SHADERS_SOURCE_FILE);

	echo -e "\treturn shader_$(SHADER_NAME);" >> $(SHADERS_SOURCE_FILE);

	echo -e "}" >> $(SHADERS_SOURCE_FILE);

endef

define write-generated-files

	@echo -e "// This file was generated by the Makefile\n" > $(SOUNDS_HEADER)
	@echo -e "#ifndef SOUND_FILES_DEF\n#define SOUND_FILES_DEF\n" >> $(SOUNDS_HEADER)
	@$(foreach file,$(SOUND_ASSETS),$(call append-sounds-header,$(file)))
	@echo -e "\n#endif" >> $(SOUNDS_HEADER)

	@echo -e "// This file was generated by the Makefile\n" > $(MODELS_HEADER)
	@echo -e "#ifndef MODELS_FILE_DEF\n#define MODELS_FILE_DEF\n" >> $(MODELS_HEADER)
	@$(foreach file,$(MODELFILES),$(call append-models-header,$(file)))
	@$(foreach file,$(ANIMFILES),$(call append-models-header-anim,$(file)))
	@echo -e "\n#endif" >> $(MODELS_HEADER)

	@echo -e "// This file was generated by the Makefile\n" > $(IMAGES_HEADER)
	@echo -e "#ifndef IMAGE_FILES_DEF\n#define IMAGE_FILES_DEF\n" >> $(IMAGES_HEADER)
	@$(foreach file,$(IMAGE_ASSETS),$(call append-images-header,$(file)))
	@echo -e "\n#endif" >> $(IMAGES_HEADER)

	@echo -e "// This file was generated by the Makefile\n" > $(SHADERS_HEADER)
	@echo -e "#ifndef SHADER_FILES_DEF\n#define SHADER_FILES_DEF\n" >> $(SHADERS_HEADER)

	@echo -e "enum {\n\tVUNIFORMS_BEGIN = -1," >> $(SHADERS_HEADER);
	$(foreach file,$(V_SHADERFILES),$(call append-shaders-header-vs,$(file)))
	@echo -e "\tNUM_VUNIFORMS" >> $(SHADERS_HEADER);
	@echo -e "};" >> $(SHADERS_HEADER);

	@echo -e "\nenum {\n\tFUNIFORMS_BEGIN = -1," >> $(SHADERS_HEADER);
	$(foreach file,$(F_SHADERFILES),$(call append-shaders-header-fs,$(file)))
	@echo -e "\tNUM_FUNIFORMS" >> $(SHADERS_HEADER);
	@echo -e "};" >> $(SHADERS_HEADER);

	@echo -e "\n\ntypedef struct {" >> $(SHADERS_HEADER)
	@echo -e "\tint program;" >> $(SHADERS_HEADER)
	@echo -e "\tint vShader;" >> $(SHADERS_HEADER)
	@echo -e "\tint fShader;" >> $(SHADERS_HEADER)
	@echo -e "\tint vUniforms[NUM_VUNIFORMS];\n\tint fUniforms[NUM_FUNIFORMS];" >> $(SHADERS_HEADER)
	@echo -e "} Shader;\n" >> $(SHADERS_HEADER)

	@echo -e "\nenum {\n\tPROGRAMS_BEGIN = -1," >> $(SHADERS_HEADER);
	@$(foreach file,$(F_SHADERFILES),$(call append-shaders-header-programs,$(file)))
	@echo -e "\tNUM_PROGRAMS" >> $(SHADERS_HEADER);
	@echo -e "};" >> $(SHADERS_HEADER);

	@echo -e "\nvoid Shaders_CompileAll(void);" >> $(SHADERS_HEADER)
	@echo -e "void Shaders_DestroyAll(void);" >> $(SHADERS_HEADER)
	@echo -e "int Shaders_GetProgram(int program);" >> $(SHADERS_HEADER)
	@echo -e "void Shaders_BindProgram(int program);" >> $(SHADERS_HEADER)
	@echo -e "int Shaders_GetVUniformLoc(int program, int uniform);" >> $(SHADERS_HEADER)
	@echo -e "int Shaders_GetFUniformLoc(int program, int uniform);" >> $(SHADERS_HEADER)
	@echo -e "int Shaders_GetProgram(int program);" >> $(SHADERS_HEADER)
	@echo -e "\n#endif" >> $(SHADERS_HEADER)

	@echo -e "// This file was generated by the Makefile\n" > $(SHADERS_SOURCE_FILE)
	@echo -e "#include <GL/glew.h>" >> $(SHADERS_SOURCE_FILE);
	@echo -e "#include \"$(notdir $(SHADERS_HEADER))\"" >> $(SHADERS_SOURCE_FILE);
	@echo -e "#include \"shaders.h\"" >> $(SHADERS_SOURCE_FILE);

	@echo -e "\nstatic Shader shaders[NUM_PROGRAMS];" >> $(SHADERS_SOURCE_FILE)

	echo -e "\nvoid Shaders_CompileAll(void){" >> $(SHADERS_SOURCE_FILE);
	@$(foreach file,$(F_SHADERFILES),$(call write-shader-c-create,$(file)))
	echo -e "}" >> $(SHADERS_SOURCE_FILE);

	echo -e "\nvoid Shaders_DestroyAll(void){" >> $(SHADERS_SOURCE_FILE);
	@$(foreach file,$(F_SHADERFILES),$(call write-shader-c-destroy,$(file)))
	echo -e "}" >> $(SHADERS_SOURCE_FILE);

	@echo -e "\nint Shaders_GetVUniformLoc(int program, int uniform){" >> $(SHADERS_SOURCE_FILE)
	echo -e "\treturn shaders[program].vUniforms[uniform];" >> $(SHADERS_SOURCE_FILE);
	echo -e "}" >> $(SHADERS_SOURCE_FILE);
	
	@echo -e "\nint Shaders_GetFUniformLoc(int program, int uniform){" >> $(SHADERS_SOURCE_FILE)
	echo -e "\treturn shaders[program].fUniforms[uniform];" >> $(SHADERS_SOURCE_FILE);
	echo -e "}" >> $(SHADERS_SOURCE_FILE);

	@echo -e "\nint Shaders_GetProgram(int program){" >> $(SHADERS_SOURCE_FILE)
	echo -e "\treturn shaders[program].program;" >> $(SHADERS_SOURCE_FILE);
	echo -e "}" >> $(SHADERS_SOURCE_FILE);

endef

IMAGE_ASSETS := $(IMAGEFILES:.png=.img)
SOUND_ASSETS := $(SOUNDFILES:.wav=.snd)

.PHONY: $(BUILD) all clean

all: $(BUILD)

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT)

$(BUILD): $(IMAGE_ASSETS) $(SOUND_ASSETS) $(V_SHADERFILES) $(F_SHADERFILES)
	@$(call write-generated-files)
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $@ -f $(CURDIR)/Makefile

else

$(OUTPUT): $(OFILES)
	@echo $(OFILES)
	$(CC) $(OFILES) -I$(SOURCES) $(LFLAGS) $(LIBS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $(INC_PATH) $(CFLAGS) $< -o $@

endif