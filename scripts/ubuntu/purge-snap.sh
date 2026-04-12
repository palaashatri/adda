#!/bin/bash

# 1. Remove all currently installed snaps
echo "Removing all snap packages..."
for snap in $(snap list | awk 'NR>1 {print $1}'); do
    sudo snap remove --purge "$snap"
done

# 2. Purge snapd and clean up directories
echo "Purging snapd daemon..."
sudo apt purge -y snapd
sudo apt autoremove -y
sudo rm -rf ~/snap /snap /var/snap /var/lib/snapd /var/cache/snapd

# 3. Prevent snapd from being reinstalled
echo "Blocking snapd via APT preferences..."
sudo tee /etc/apt/preferences.d/nosnap.pref <<EOF
Package: snapd
Pin: release a=*
Pin-Priority: -10
EOF

# 4. Set up Mozilla APT repository for Firefox .deb
echo "Setting up official Mozilla APT repository..."
sudo install -d -m 0755 /etc/apt/keyrings
wget -q https://packages.mozilla.org/apt/repo-signing-key.gpg -O- | sudo tee /etc/apt/keyrings/packages.mozilla.org.asc > /dev/null

# Verify key fingerprint (optional but recommended)
echo "deb [signed-by=/etc/apt/keyrings/packages.mozilla.org.asc] https://packages.mozilla.org/apt mozilla main" | sudo tee /etc/apt/sources.list.d/mozilla.list > /dev/null

# 5. Prioritize Mozilla repository over Ubuntu's transitional package
sudo tee /etc/apt/preferences.d/mozilla <<EOF
Package: *
Pin: origin packages.mozilla.org
Pin-Priority: 1000
EOF

# 6. Install Firefox .deb
echo "Installing Firefox .deb..."
sudo apt update
sudo apt install -y firefox

echo "Success! Snap has been purged and Firefox .deb is installed."
