# Joseph Erlinger (jerling2@uoregon.edu)

TARBALL_NAME = joseph_erlinger_DebianUTM_Project3.tar.gz
BUILD_NAME = joseph_erlinger_DebianUTM_Project3
PARTS = part1

.PHONY : all clean tarball

all :
	@$(MAKE) -C $(PARTS)

clean :
	@$(MAKE) -C $(PARTS) clean
	@rm -f $(TARBALL_NAME)
	@rm -rf $(BUILD_NAME)

tarball : clean
	tar zcvf $(TARBALL_NAME) --transform 's,^,$(BUILD_NAME)/,' $(PARTS)

stage : clean
	@git add .
	@git status

unstage : clean
	@git restore --staged .
	@echo unstaged changes for commit