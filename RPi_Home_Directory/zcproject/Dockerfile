# 1. Use an official, lightweight Python base image for ARM architectures
FROM python:3.11-slim

# 2. Set the working directory inside the container
WORKDIR /app

# 3. Copy the requirements file first to leverage Docker's caching mechanism
COPY requirements.txt .

# 4. Install the required Python packages
RUN pip install --no-cache-dir -r requirements.txt

# 5. Copy the bridge script into the container (do NOT include credentials)
COPY bridge.py .

# Note: Do not copy `firebase_credentials.json` into the image. Provide
# credentials at runtime via environment variables, Docker secrets, or
# a cloud secret manager.

# 6. Run Python in unbuffered mode (-u) so logs stream to Docker instantly
CMD ["python", "-u", "bridge.py"]
