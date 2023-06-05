PO = $(NAME).po
FORMAT = po
export $(FORMAT)

CA ?= cadius-145.exe

# Unix or Windows
ifeq ($(shell echo),)
	CP = cp $1
	MV = mv
	RM = rm
else
	CP = copy $(subst /,\,$1)
	MV = ren
	RM = del
endif

REMOVES += $(PO)

.PHONY: po
po: $(PO)

$(NAME).system:
	$(call CP, $(subst \,/,$(shell cl65 --print-target-path)/apple2/util/loader.system) $(NAME).system#FF2000)

$(PO): $(NAME).apple2 $(NAME).system
	$(call CP, apple2/template.po $@)
	$(CP) $(NAME).apple2 $(NAME)#061000
	$(CA) addfile $(NAME).po /$(NAME) $(NAME).system#FF2000
	$(CA) addfile $(NAME).po /$(NAME) $(NAME)#061000
	$(CA) addfolder $(NAME).po /$(NAME)/PT3 PT3
	$(RM) $(NAME).system#FF2000
	$(RM) $(NAME)#061000