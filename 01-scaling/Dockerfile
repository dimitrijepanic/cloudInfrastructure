# Base Docker Image
FROM python:2

# Working Directory
WORKDIR /container

# Dependencies 
RUN pip install delegator.py 
RUN pip install web.py

# Add Python file to working directory
ADD pdfmagic/pdfmagic.py /container

# Expose container's port to clients  
EXPOSE 8080

# Command that runs on the container
CMD ["python", "/container/pdfmagic.py"]

