# Linux-HW
* linux-kernel-version:6.7.4
* busybox-1.36.1

## How to run:
```bash
./build.sh
```

```bash
insmod lib/modules/6.7.4/phonebook.ko
```
#### dmesg:
```
[   10.307057] Phonebook: Initializing phonebook module
[   10.307084] Phonebook: Device created successfully
```

## Tests:
### Add users:
```bash
echo "add_user John Doe 30 123456789 john@doe.com" > /dev/phonebook
echo "add_user John Wick 40 6666666 john@wick.com" > /dev/phonebook
echo "add_user John Smith 355 61626 john@smith.com" > /dev/phonebook
```
#### dmesg:
```
[   10.538403] Phonebook: Device opened
[   10.538413] Phonebook: Added User John Doe
[   10.538415] Phonebook: Device successfully closed
[   10.540117] Phonebook: Device opened
[   10.540118] Phonebook: Added User John Wick
[   10.540119] Phonebook: Device successfully closed
[   11.742069] Phonebook: Device opened
[   11.742076] Phonebook: Added User John Smith
[   11.742078] Phonebook: Device successfully closed
```
### List all users:
```bash
~ cat /dev/phonebook
Name: John Doe
Age: 30
Phone: 123456789
Email: john@doe.com

Name: John Wick
Age: 40
Phone: 6666666
Email: john@wick.com

Name: John Smith
Age: 355
Phone: 61626
Email: john@smith.com

```

### Get user:
```bash
echo "get_user Wick" > /dev/phonebook
```
#### dmesg:
```
[   64.286482] Phonebook: Device opened
[   64.286489] Phonebook: Found User: John Wick, Age: 40, Phone: 6666666, Email: john@wick.com
[   64.286491] Phonebook: Device successfully closed
```

### Del user:
```bash
echo "del_user John Smith" > /dev/phonebook
```
#### dmesg:
```
[  111.281016] Phonebook: Device opened
[  111.281045] Phonebook: Deleting User John Smith
[  111.281049] Phonebook: Device successfully closed
```

```bash
~ cat /dev/phonebook
Name: John Doe
Age: 30
Phone: 123456789
Email: john@doe.com

Name: John Wick
Age: 40
Phone: 6666666
Email: john@wick.com
```