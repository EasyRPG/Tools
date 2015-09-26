
PREFIX=/usr/local

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/thumbnailers
	mkdir -p $(DESTDIR)$(PREFIX)/share/mime/packages
	install -m755 xyz-thumbnailer.sh $(DESTDIR)$(PREFIX)/bin/xyz-thumbnailer
	install -m644 integration/xyz.thumbnailer $(DESTDIR)$(PREFIX)/share/thumbnailers
	install -m644 integration/image-xyz.xml $(DESTDIR)$(PREFIX)/share/mime/packages
ifeq ($(strip $(DESTDIR)),)
	update-mime-database $(PREFIX)/share/mime
else
	@echo "Not updating mime database, because a destination directory is specified."
	@echo "Do not forget to call 'update-mime-database $(PREFIX)/share/mime' after installation"
endif
