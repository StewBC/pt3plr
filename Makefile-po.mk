PO = $(NAME).po

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

$(PO): $(NAME).apple2
	$(call CP, apple2/template.po $@)
	$(CP) $(NAME).apple2 $(NAME).system#FF2000
	$(CA) addfile $(NAME).po /$(NAME) $(NAME).system#FF2000
	$(CA) addfolder $(NAME).po /$(NAME)/PT3 PT3
	$(RM) $(NAME).system#FF2000
