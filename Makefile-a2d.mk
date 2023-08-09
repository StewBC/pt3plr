A2D = $(NAME)-a2d.po

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

REMOVES += $(A2D)

.PHONY: a2d
a2d: $(A2D)

$(A2D): $(NAME).po
	$(CA) deletefile $(NAME).po /$(NAME)/$(NAME).system
	$(CA) extractfile $(NAME).po /$(NAME)/$(NAME) .
	$(CA) deletefile $(NAME).po /$(NAME)/$(NAME)
	$(MV) $(NAME)#062000 basis.system#FF2000
	$(CA) addfile $(NAME).po /$(NAME) basis.system#FF2000
	$(RM) basis.system#FF2000
