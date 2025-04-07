# Packaging Myrlyn with OBS

## Checking Out the Package

Checking out into a branch package:

```
cd /work/obs
osc bco system:packagemanager myrlyn
cd system:packagemanager/myrlyn
```

Checking out the real thing (Caution!):

```
cd /work/obs
osc co system:packagemanager myrlyn
cd system:packagemanager/myrlyn
```


## Updating the Git Tag

```
vi _service
```

Edit this line to the current tag:

```
    <param name="revision">0.42.1</param>
```

## Run the Services


```
osc service runall
```

This checks out the tarball of the specified tag (from _service), recompresses
it, creates a cpio archive from it, and regenerates the change log from the git
commits.

You should now get new files

- `myrlyn-0.42.2.obscpio`   (Important!)
- `myrlyn-0.42.2.tar.zst`   (Temporary - remove!)

```
rm *.zst
```


## Check in the new cpio Archive

```
osc ar 
```

## Double-Check

```
osc status
```

## Check In

```
osc ci
```

## Check the Result

- In the home project: https://build.opensuse.org/
  - Left sidebar: "Your Home Project"
  - Center tab: "Subprojects"
  - `home:${user}:branches:system:packagemanager`
  - "myrlyn"